# FT232RL Red Panda Build Instructions

## Overview

This document provides step-by-step instructions for building and deploying the FT232RL Red Panda implementation on TC275 and PC platforms.

## Prerequisites

### Hardware Requirements

#### TC275 Development Board:
- **TC275** or **TC375** TriCore development board
- **CAN transceivers** (TJA1050, SN65HVD230, etc.)
- **FT232RL breakout board** or development board
- **Logic level converter** (if voltage mismatch)
- **Breadboard and jumper wires**

#### Tools:
- **Oscilloscope** (recommended for debugging)
- **CAN analyzer** (optional, for validation)
- **Multimeter**

### Software Requirements

#### TC275 Development:
- **AURIX Development Studio** (Eclipse-based IDE)
- **TASKING Compiler** or **GCC TriCore**
- **iLLD (Infineon Low Level Drivers)**
- **FreeRTOS** (if not included in your project)

#### PC Development:
- **Python 3.7+** with pip
- **C/C++ compiler** (optional, for C wrapper)
- **Serial terminal** (PuTTY, screen, etc.)

## Step 1: Hardware Setup

### FT232RL to TC275 Connection

```
FT232RL Board    TC275 Board
--------------   -----------
VCC (3.3V)   →   3V3
GND          →   GND
TXD          →   P14.1 (ASC0_RX)
RXD          ←   P14.0 (ASC0_TX)
RTS          →   P14.3 (ASC0_CTS) [optional]
CTS          ←   P14.2 (ASC0_RTS) [optional]
```

### Voltage Level Considerations:
- **FT232RL**: 3.3V or 5V I/O (configurable)
- **TC275**: 3.3V I/O
- **Use level converter** if FT232RL is configured for 5V

### CAN Bus Connections:
```
TC275 Board      CAN Transceiver    CAN Bus
-----------      ---------------    -------
P20.7 (CAN0_TX) → TXD               
P20.8 (CAN0_RX) ← RXD               
3V3             → VCC               
GND             → GND               
                  CANH            → CAN High
                  CANL            → CAN Low
```

## Step 2: TC275 Firmware Build

### 2.1 Project Setup

1. **Create new AURIX project:**
   ```
   File → New → AURIX Project
   - Target: TC275
   - Compiler: TASKING or GCC
   - Include: iLLD, FreeRTOS
   ```

2. **Copy FT232RL implementation:**
   ```bash
   # Copy headers to your project
   cp TC275/*.h /path/to/your/project/src/
   cp Common/*.h /path/to/your/project/src/
   ```

3. **Add to build system:**
   ```makefile
   # Add to Makefile or project settings
   INCLUDES += -Isrc/Common -Isrc/TC275
   SOURCES += src/ft232rl_tc275.c src/ft232rl_can_integration.c
   ```

### 2.2 Configuration

1. **UART Configuration (main.c):**
   ```c
   #include "ft232rl_tc275.h"
   
   int main(void) {
       // Initialize system
       initSystem();
       
       // Initialize FT232RL
       if (!FT232RL_TC275_Init()) {
           // Handle error
           return -1;
       }
       
       // Initialize CAN
       if (!FT232RL_CAN_Init()) {
           // Handle error
           return -1;
       }
       
       // Start FT232RL tasks
       FT232RL_TC275_Start();
       
       // Start FreeRTOS scheduler
       vTaskStartScheduler();
       
       return 0;
   }
   ```

2. **Pin Configuration (Configuration.h):**
   ```c
   // UART pin configuration
   #define FT232RL_UART_TX_PIN    IfxAsclin0_TX_P14_0_OUT
   #define FT232RL_UART_RX_PIN    IfxAsclin0_RX_P14_1_IN
   #define FT232RL_UART_RTS_PIN   IfxAsclin0_RTS_P14_2_OUT
   #define FT232RL_UART_CTS_PIN   IfxAsclin0_CTS_P14_3_IN
   
   // CAN pin configuration
   #define CAN0_TX_PIN            IfxCan_TXD00_P20_7_OUT
   #define CAN0_RX_PIN            IfxCan_RXD00_P20_8_IN
   ```

3. **FreeRTOS Configuration (FreeRTOSConfig.h):**
   ```c
   #define configUSE_PREEMPTION                1
   #define configUSE_IDLE_HOOK                 0
   #define configUSE_TICK_HOOK                 0
   #define configCPU_CLOCK_HZ                  300000000  // 300 MHz
   #define configTICK_RATE_HZ                  1000       // 1 kHz
   #define configMAX_PRIORITIES                8
   #define configMINIMAL_STACK_SIZE            128
   #define configTOTAL_HEAP_SIZE               32768      // 32 KB
   ```

### 2.3 Implementation Files

Create the following C implementation files based on the headers:

1. **ft232rl_tc275.c** - Main TC275 implementation
2. **ft232rl_can_integration.c** - CAN system integration
3. **ft232rl_protocol.c** - Protocol handling functions

### 2.4 Build Process

1. **Compile:**
   ```bash
   # TASKING Compiler
   ctc -o firmware.elf src/*.c -ltasking_rt
   
   # Or GCC TriCore
   tricore-gcc -o firmware.elf src/*.c -lfreertos
   ```

2. **Generate HEX:**
   ```bash
   ctc -f intel-hex firmware.elf -o firmware.hex
   ```

3. **Flash to TC275:**
   ```bash
   # Using UDE debugger or DAS
   ./flash_tc275.sh firmware.hex
   ```

## Step 3: PC Software Setup

### 3.1 Python Environment

1. **Install Python dependencies:**
   ```bash
   pip install pyserial
   pip install numpy  # Optional, for data analysis
   ```

2. **Install FT232RL drivers:**
   - **Windows**: Download from FTDI website
   - **Linux**: Usually included (ftdi_sio module)
   - **macOS**: Download from FTDI website

### 3.2 Python Wrapper Installation

1. **Copy Python files:**
   ```bash
   cp PC/ft232rl_python_wrapper.py /path/to/your/project/
   ```

2. **Test connection:**
   ```python
   from ft232rl_python_wrapper import Panda
   
   # Test auto-detection
   try:
       panda = Panda()
       print(f"Connected to: {panda.port}")
       panda.close()
   except Exception as e:
       print(f"Connection failed: {e}")
   ```

### 3.3 C/C++ Wrapper (Optional)

1. **Compile C wrapper:**
   ```bash
   gcc -o ft232rl_test ft232rl_pc_adapter.c -lserial
   ```

2. **Link with your application:**
   ```c
   #include "ft232rl_pc_adapter.h"
   
   FT232RL_PC_Context_t ctx;
   FT232RL_PC_Init(&ctx, "COM3", 3000000);
   FT232RL_PC_Connect(&ctx);
   ```

## Step 4: Testing and Validation

### 4.1 Basic Connectivity Test

1. **Check UART communication:**
   ```bash
   # Linux
   screen /dev/ttyUSB0 3000000
   
   # Windows (PuTTY)
   # Set COM port, 3000000 baud, 8N1
   ```

2. **Test frame exchange:**
   ```python
   from ft232rl_python_wrapper import Panda
   
   panda = Panda("COM3")  # Adjust port
   version = panda.get_version()
   print(f"Firmware version: {version}")
   ```

### 4.2 CAN Communication Test

1. **Run example test:**
   ```bash
   cd Examples/
   python simple_can_test.py COM3
   ```

2. **Monitor with oscilloscope:**
   - Check UART signals (TXD/RXD)
   - Verify CAN bus signals (CANH/CANL)
   - Measure timing and verify no corruption

### 4.3 Performance Testing

1. **Throughput test:**
   ```python
   # Test maximum CAN message rate
   import time
   from ft232rl_python_wrapper import Panda, CANMessage
   
   panda = Panda()
   start_time = time.time()
   
   # Send 1000 messages
   messages = [CANMessage(0x123 + i, b'\x01\x02\x03\x04', 0) for i in range(1000)]
   sent = panda.can_send_many(messages)
   
   duration = time.time() - start_time
   print(f"Sent {sent} messages in {duration:.2f}s ({sent/duration:.1f} msg/s)")
   ```

2. **Latency test:**
   ```python
   # Measure round-trip latency
   start = time.time()
   panda.send_heartbeat()
   latency = time.time() - start
   print(f"Heartbeat latency: {latency*1000:.1f}ms")
   ```

## Step 5: Integration with Existing Tools

### 5.1 Red Panda Tools Compatibility

1. **Modify import statements:**
   ```python
   # Original
   from panda import Panda
   
   # Modified for FT232RL
   from ft232rl_python_wrapper import Panda
   ```

2. **Test with comma.ai tools:**
   ```bash
   # Clone openpilot tools
   git clone https://github.com/commaai/openpilot.git
   
   # Modify panda import in relevant scripts
   # Replace: from panda import Panda
   # With: from ft232rl_python_wrapper import Panda
   ```

### 5.2 Custom Application Integration

1. **Replace Panda USB with FT232RL:**
   ```python
   class MyCANTool:
       def __init__(self, use_ft232rl=False, port=None):
           if use_ft232rl:
               from ft232rl_python_wrapper import Panda
               self.panda = Panda(port)
           else:
               from panda import Panda
               self.panda = Panda()
   ```

## Step 6: Troubleshooting

### 6.1 Common Hardware Issues

**No UART communication:**
- Check wiring (TX↔RX, RX↔TX)
- Verify voltage levels (3.3V)
- Test with lower baud rate (115200)
- Check ground connection

**CAN communication fails:**
- Verify CAN transceiver connections
- Check CAN bus termination (120Ω)
- Measure CAN voltage levels
- Test with CAN analyzer

### 6.2 Common Software Issues

**Frame synchronization errors:**
```c
// Add debug output to TC275
#ifdef FT232RL_DEBUG
printf("Frame sync error: got 0x%02X, expected 0xAA\n", sync_byte);
#endif
```

**Checksum failures:**
```python
# Debug checksum calculation in Python
def debug_checksum(data):
    checksum = 0
    for byte in data:
        checksum ^= byte
    print(f"Calculated checksum: 0x{checksum:02X}")
    return checksum
```

**Timeout issues:**
```c
// Increase timeouts for debugging
#define FT232RL_UART_TIMEOUT_MS    1000  // Instead of 100
```

### 6.3 Performance Optimization

**Reduce latency:**
1. Enable hardware flow control
2. Optimize task priorities
3. Reduce FreeRTOS tick rate overhead
4. Use DMA for UART transfers

**Increase throughput:**
1. Optimize frame packing
2. Implement data compression
3. Use larger frame sizes
4. Pipeline transfers

## Step 7: Production Deployment

### 7.1 Code Optimization

1. **Remove debug code:**
   ```c
   #ifdef DEBUG
   // Remove all debug prints
   #endif
   ```

2. **Optimize compiler settings:**
   ```makefile
   CFLAGS += -O2 -DNDEBUG
   ```

### 7.2 Testing

1. **Extended testing:**
   - 24+ hour stress test
   - Temperature cycling
   - EMC/EMI testing
   - Real vehicle validation

2. **Error handling:**
   - Watchdog implementation
   - Automatic recovery
   - Error logging

### 7.3 Documentation

1. **Create user manual**
2. **Document pinout and connections**
3. **Provide troubleshooting guide**
4. **Include performance specifications**

## Conclusion

This build guide provides a complete implementation path for FT232RL Red Panda on TC275. While the performance is lower than native USB Red Panda, it offers excellent compatibility and sufficient performance for most automotive CAN applications.

For maximum performance, consider upgrading to FT232H (12 Mbps) or implementing the Ethernet bridge solution documented elsewhere in this project.

**Good luck with your build!** 🔧⚡