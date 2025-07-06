# Red Panda Communication System

## Overview
The Red Panda is a comprehensive automotive communication bridge that connects computers to vehicle CAN networks through an intelligent USB-C harness system and STM32H7-based CAN-FD controllers.

## Complete System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                           RED PANDA COMMUNICATION SYSTEM                        │
└─────────────────────────────────────────────────────────────────────────────────┘

Computer/Laptop                Red Panda Device                Vehicle Network
┌─────────────────┐           ┌─────────────────────┐          ┌─────────────────┐
│                 │           │                     │          │                 │
│   Python App    │           │      STM32H7        │          │   Vehicle ECUs  │
│   ┌─────────────┤           │   ┌─────────────┐   │          │   ┌─────────────┤
│   │ panda lib   │    USB    │   │   USB OTG   │   │   USB-C  │   │ Engine ECU  │
│   │             │◄──────────┤   │ Controller  │   │◄─────────┤   │Transmission │
│   │ CAN msgs    │   2.0     │   │  (A11/A12)  │   │ Harness  │   │ Brake ECU   │
│   │ Control cmds│  12 Mbps  │   │             │   │  Box     │   │ Body ECU    │
│   └─────────────┤           │   └─────────────┘   │          │   └─────────────┤
│                 │           │          │          │          │                 │
│   ┌─────────────┤           │   ┌─────────────┐   │          │   ┌─────────────┤
│   │WebUSB/WinUSB│           │   │FDCAN1 (B8/9)│   │   CAN    │   │   OBD-II    │
│   │   Support   │           │   │FDCAN2(B5/6 │   │◄─────────┤   │    Port     │
│   └─────────────┤           │   │   or B12/13)│   │ 125kbps- │   │  (Pin 6,14) │
│                 │           │   │FDCAN3(G9/10)│   │  5 Mbps  │   └─────────────┤
└─────────────────┘           │   └─────────────┘   │          │                 │
                              │          │          │          │                 │
                              │   ┌─────────────┐   │          │                 │
                              │   │CAN Trans-   │   │          │                 │
                              │   │ceivers 1,2,4│   │          │                 │
                              │   │(G11,B3,B4)  │   │          │                 │
                              │   └─────────────┘   │          │                 │
                              └─────────────────────┘          └─────────────────┘
```

## Physical Connection Chain

```
Vehicle OBD-II Port → USB-C Harness Box → USB-C Cable → Red Panda USB-C Port → Computer USB Port
```

### Connection Details:
1. **Vehicle OBD-II Port**: Standard 16-pin automotive diagnostic connector
2. **USB-C Harness Box**: Converts OBD-II signals to USB-C interface with electrical isolation
3. **USB-C Cable**: Carries CAN signals, power, and harness detection signals
4. **Red Panda**: STM32H7-based bridge device with CAN-FD controllers
5. **Computer**: Runs Python applications for vehicle communication

## FDCAN Controller Usage Analysis

### Available vs Used Controllers:
```
STM32H7 FDCAN Controllers:
┌─────────────────────────────────────────────────────────────┐
│ FDCAN1 (B8/B9)    │ ✅ ACTIVELY USED    │ CAN Bus 0        │
│ FDCAN2 (B5/B6 or  │ ✅ ACTIVELY USED    │ CAN Bus 1        │
│        B12/B13)   │    (Multiplexed)    │ (OBD-II/Harness) │
│ FDCAN3 (G9/G10)   │ ⚠️  CONFIGURED      │ CAN Bus 2        │
│                   │    BUT UNUSED       │ (No Transceiver) │
└─────────────────────────────────────────────────────────────┘
```

### Why Only 2 of 3 FDCAN Controllers Are Used:
- **FDCAN1**: Direct vehicle CAN access
- **FDCAN2**: OBD-II/harness connection with orientation switching
- **FDCAN3**: Configured but no physical transceiver (cost optimization)

## USB-C Harness System

### Harness Detection Mechanism:
```
USB-C Connector SBU Pins:
┌─────────────────────────────────────────────────────┐
│ SBU1 (GPIO C4) ──→ ADC Channel 4  ──→ Voltage Mon  │
│ SBU2 (GPIO A1) ──→ ADC Channel 17 ──→ Voltage Mon  │
└─────────────────────────────────────────────────────┘

Harness Status Detection:
┌────────────────┬─────────────┬──────────────────────┐
│ Status         │ SBU Voltages│ CAN Pin Routing     │
├────────────────┼─────────────┼──────────────────────┤
│ NORMAL         │ SBU2 < SBU1 │ FDCAN2 → B5/B6      │
│ FLIPPED        │ SBU1 < SBU2 │ FDCAN2 → B12/B13    │
│ NOT_CONNECTED  │ Both > 1.65V│ No routing           │
└────────────────┴─────────────┴──────────────────────┘
```

### Harness Orientation Handling:
```
Normal Orientation:                    Flipped Orientation:
┌─────────────────┐                   ┌─────────────────┐
│ B5/B6 pins      │ ◄─── FDCAN2       │ B12/B13 pins    │ ◄─── FDCAN2
│ Transceiver 2   │      active       │ Transceiver 4   │      active
│ (GPIO B3)       │                   │ (GPIO B4)       │
└─────────────────┘                   └─────────────────┘
```

## USB 2.0 Communication Interface

### USB Physical Layer:
- **D+ Pin**: A12 (PA12) - USB Data Plus
- **D- Pin**: A11 (PA11) - USB Data Minus
- **Protocol**: USB 2.0 Full Speed (12 Mbps)
- **Function**: USB OTG FS (GPIO_AF10_OTG1_FS)

### USB Device Identity:
```
USB Device Descriptor:
├── Vendor ID (VID): 0x3801 (comma.ai)
├── Product ID (PID): 0xDDCC (app) / 0xDDEE (bootloader)
├── Manufacturer: "comma.ai"
├── Product: "panda"
└── Serial Number: Derived from chip UID
```

### USB Endpoint Configuration:
```
USB Endpoints:
┌────────────┬─────────────┬─────────────────────────────────┐
│ Endpoint   │ Direction   │ Purpose                         │
├────────────┼─────────────┼─────────────────────────────────┤
│ EP0        │ Bidirectional│ Control transfers, device setup │
│ EP1 (IN)   │ Device→Host │ CAN messages from vehicle       │
│ EP2 (OUT)  │ Host→Device │ UART/Serial data forwarding     │
│ EP3 (OUT)  │ Host→Device │ CAN messages to vehicle         │
└────────────┴─────────────┴─────────────────────────────────┘
```

## CAN Bus Communication Flow

### Message Transmission Path:
```
1. Python Application
   ↓ (USB control command)
2. Red Panda USB Controller
   ↓ (Internal processing)
3. Safety Hook Validation
   ↓ (Safety check passed)
4. CAN TX Queue
   ↓ (FDCAN interrupt)
5. FDCAN Controller
   ↓ (Hardware FIFO)
6. CAN Transceiver
   ↓ (Differential signals)
7. Vehicle CAN Bus
```

### CAN Message Structure:
```c
typedef struct {
    unsigned int addr : 29;        // CAN ID (11-bit or 29-bit)
    unsigned char data[64];        // Data payload (8 bytes CAN, up to 64 bytes CAN-FD)
    unsigned char data_len_code;   // Length encoding (0-15)
    unsigned char extended : 1;    // Extended frame flag
    unsigned char fd : 1;          // CAN-FD frame flag
    unsigned char brs : 1;         // Bit Rate Switch flag
    unsigned char bus : 3;         // CAN bus number (0-2)
    unsigned char checksum;        // Packet integrity check
} CANPacket_t;
```

## CAN Transceiver Control

### Transceiver Mapping:
```
CAN Transceivers:
┌─────────────┬─────────────┬─────────────┬─────────────────┐
│ Transceiver │ Control Pin │ Usage       │ Status          │
├─────────────┼─────────────┼─────────────┼─────────────────┤
│ 1           │ GPIO G11    │ FDCAN1      │ Active          │
│ 2           │ GPIO B3     │ FDCAN2      │ Normal harness  │
│ 3           │ GPIO D7     │ FDCAN3      │ Unused          │
│ 4           │ GPIO B4     │ FDCAN2      │ Flipped harness │
└─────────────┴─────────────┴─────────────┴─────────────────┘
```

### Power Management:
- **Active Low Control**: Transceivers enabled when GPIO = 0
- **Selective Activation**: Only required transceivers enabled
- **Power Save Mode**: Unused transceivers disabled
- **Ignition Awareness**: Main bus stays active for ignition detection

## Harness Relay System

### Intercept Functionality:
```
Relay Control System:
┌─────────────────────────────────────────────────────────┐
│ Pass-Through Mode:                                      │
│ Vehicle ←→ OBD-II ←→ Red Panda (Monitor Only)          │
│                                                         │
│ Intercept Mode:                                         │
│ Vehicle ←→ Red Panda ←→ OBD-II (Control/Filter)        │
└─────────────────────────────────────────────────────────┘

Relay Pins:
├── SBU1 Relay: GPIO C10 (Controls intercept path)
└── SBU2 Relay: GPIO C11 (Controls ignition path)
```

## Advanced Features

### CAN-FD Capabilities:
```
CAN-FD Features:
┌─────────────────┬─────────────────────────────────────┐
│ Nominal Rate    │ Up to 500 kbps                      │
│ Data Phase Rate │ Up to 5 Mbps (with BRS)            │
│ Payload Size    │ Up to 64 bytes (vs 8 bytes std CAN)│
│ Error Handling  │ Separate data phase error detection │
│ Auto Detection  │ Dynamic CAN-FD capability detection │
│ Non-ISO Mode    │ Support for non-ISO CAN-FD         │
└─────────────────┴─────────────────────────────────────┘
```

### Safety Integration:
```
Safety Model Flow:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   CAN Message   │───→│  Safety Hook    │───→│  Hardware TX    │
│   from Host     │    │  Validation     │    │  (if approved)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                               │
                               ▼ (if rejected)
                       ┌─────────────────┐
                       │  Message        │
                       │  Dropped        │
                       └─────────────────┘
```

## Communication Protocol Summary

### Why Both CAN and USB Are Necessary:

#### **Protocol Bridge Function:**
```
Computer Domain        │        Vehicle Domain
─────────────────────────────────────────────────
USB Protocol          │        CAN Protocol
12 Mbps               │        125 kbps - 5 Mbps
Single-ended signals  │        Differential signals
5V logic levels       │        12V automotive levels
Packet-based          │        Frame-based
```

#### **Data Flow Examples:**
```python
# Host sends CAN message via USB
panda.can_send(0x123, b'data', 0)
  ↓ USB command to Red Panda
  ↓ Safety validation
  ↓ FDCAN1 transmission
  ↓ Vehicle CAN bus

# Vehicle sends CAN message
Vehicle CAN bus
  ↓ FDCAN1 reception
  ↓ USB bulk transfer
  ↓ Python library
msgs = panda.can_recv()
```

## System Health Monitoring

### Health Packet Information:
```
Health Monitoring:
├── Harness Status: NC/NORMAL/FLIPPED
├── SBU Voltages: Real-time ADC measurements
├── Ignition State: Vehicle on/off detection
├── CAN Bus Health: Error counters per bus
├── Voltage/Current: 12V supply monitoring
├── Safety Mode: Current safety configuration
├── Error Statistics: TX/RX error tracking
└── Interrupt Load: System performance metrics
```

## Power System

### Power Sources:
```
Power Input Options:
┌─────────────────┬─────────────────┬─────────────────┐
│ Source          │ Voltage         │ Usage           │
├─────────────────┼─────────────────┼─────────────────┤
│ Vehicle 12V     │ 12V (via OBD-II│ Normal operation│
│ USB Bus Power   │ 5V (via USB)    │ Bench testing   │
│ External Supply │ 5V-12V range    │ Development     │
└─────────────────┴─────────────────┴─────────────────┘
```

The Red Panda's communication system provides a comprehensive bridge between modern computing environments and automotive CAN networks, enabling advanced vehicle integration, reverse engineering, and diagnostic capabilities through its intelligent harness system and high-performance CAN-FD controllers.