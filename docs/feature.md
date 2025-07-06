# Red Panda Software Features

## Overview
The Red Panda is the most advanced panda variant, featuring STM32H7 microcontroller with comprehensive CAN-FD support and intelligent harness management for modern automotive applications.

## Hardware Platform
- **MCU**: STM32H7 (STM32H725/735 series)
- **Architecture**: ARM Cortex-M7 high-performance processor
- **Hardware Type ID**: `HW_TYPE_RED_PANDA` (0x07)
- **Board Detection**: GPIO-based identification (F7-F10 pins)

## CAN/CAN-FD Communication
### Core CAN Features
- **CAN-FD Native Support**: Full hardware CAN-FD capability
- **CAN Controller**: 3x FDCAN peripherals (FDCAN1, FDCAN2, FDCAN3)
- **CAN Bus Count**: 3 independent CAN buses
- **CAN Transceivers**: 4 individually controlled transceivers
  - Transceiver 1: GPIO G11 control
  - Transceiver 2: GPIO B3 control  
  - Transceiver 3: GPIO D7 control
  - Transceiver 4: GPIO B4 control

### Advanced CAN-FD Features
- **Bit Rate Switch (BRS)**: High-speed data phase support
- **Non-ISO CAN-FD**: Compatibility with non-ISO CAN-FD implementations
- **Automatic CAN-FD Detection**: Dynamic protocol detection
- **Enhanced Error Handling**: Separate data phase error reporting
- **Speed Support**: Up to 500 kbps nominal, 2000 kbps data phase
- **Hardware Message Filtering**: Efficient message filtering
- **TX/RX FIFO**: Buffer management with overflow protection
- **Error Statistics**: Comprehensive error counters and bus health monitoring

## Harness System
### Intelligent Harness Management
- **Harness Detection**: Real-time USB-C orientation detection
- **SBU Pin Configuration**:
  - SBU1: GPIO C4 (ADC channel 4)
  - SBU2: GPIO A1 (ADC channel 17)
  - Relay SBU1: GPIO C10
  - Relay SBU2: GPIO C11
- **Harness Status Types**:
  - `HARNESS_STATUS_NORMAL`: Standard orientation
  - `HARNESS_STATUS_FLIPPED`: Reversed orientation
  - `HARNESS_STATUS_NC`: No connection
- **Dynamic CAN Routing**: Automatic CAN pin switching based on harness orientation
- **Relay Control**: Hardware relay switching for intercept functionality

## Interface Capabilities
### USB Interface
- **High-Speed USB 2.0**: STM32H7 USB OTG support
- **Vendor ID**: 0x3801
- **Product IDs**: 0xDDCC (application), 0xDDEE (bootloader)
- **Bulk Transfer**: High-throughput CAN data communication
- **Power Management**: USB load switch control and 5V output sensing

### Debug and Development
- **SWD Debug**: In-circuit debugging support
- **UART Interfaces**: Multiple UART channels for debugging
- **DFU Mode**: Device Firmware Update for recovery
- **Debug Console**: Real-time debugging and monitoring

## Power Management
### Voltage and Power Control
- **12V Automotive Voltage**: Real-time voltage monitoring with 11x scaling
- **Smart Power Saving**: Comprehensive power management
- **Transceiver Power Control**: Individual CAN transceiver enable/disable
- **USB Power Management**: Load switch control and power mode detection
- **Ignition Detection**: Harness-based ignition line monitoring

## LED System
### Visual Indicators
- **3-Color LED System**: Red, Green, Blue LEDs
- **LED GPIOs**:
  - Red LED: GPIO E4
  - Green LED: GPIO E3
  - Blue LED: GPIO E2
- **Open-Drain Control**: Individual LED control
- **Status Indication**: System status and communication indicators

## Safety Features
### Safety Model Integration
- **opendbc Safety Models**: Full integration with automotive safety models
- **OBD-II Support**: Complete OBD-II safety mode support
- **Ignition Safety**: Harness-based ignition detection
- **Relay Safety**: Hardware relay control for safety intercept
- **Message Validation**: CRC-based checksum validation
- **Error Recovery**: Automatic bus-off recovery and error handling

## Firmware Features
### Secure Firmware
- **Dual-Stage Bootloader**: Secure firmware update mechanism
- **Digital Signature**: RSA-1024 signed firmware with verification
- **Version Control**: Git-based version tracking
- **Secure Boot**: Verified boot process

### Health Monitoring
- **Comprehensive Health Reporting**:
  - CAN bus health per channel
  - Error statistics and counters
  - Voltage and current monitoring
  - Interrupt load measurement
  - SBU voltage monitoring
  - System temperature monitoring

## Advanced Features
### Real-Time Processing
- **Interrupt Management**: Sophisticated real-time CAN processing
- **DMA Buffers**: Large DMA buffers for high-throughput communication
- **Error Recovery**: Automatic error recovery and bus-off handling
- **Diagnostic Capabilities**: Extensive debugging and diagnostic features

### CAN Mode Switching
- **Harness-Aware Routing**: Intelligent CAN pin assignment
- **Normal Mode**: Standard CAN operation
- **OBD-II Mode**: `CAN_MODE_OBD_CAN2` support
- **Dynamic Reconfiguration**: GPIO reconfiguration based on harness status

## Features Not Available
The following features are not supported on Red Panda:
- **SPI Interface**: No SPI support (`has_spi = false`)
- **Fan Control**: No fan control capabilities
- **IR Power**: No infrared power control
- **Siren/Audio**: No siren or audio capabilities
- **Current Measurement**: No current measurement functionality
- **Bootkick**: No bootkick functionality
- **SOM GPIO**: No System-on-Module GPIO reading
- **Amplifier Control**: No amplifier control

## Unique Advantages
1. **CAN-FD Native Support**: Hardware-accelerated CAN-FD unlike F4-based variants
2. **Higher Performance**: STM32H7 provides superior processing power
3. **Advanced Harness System**: Sophisticated harness detection and routing
4. **Enhanced Power Management**: Granular power control and monitoring
5. **Superior Error Handling**: Advanced CAN error detection and recovery
6. **Larger Memory**: More flash and RAM for complex safety models
7. **Modern Architecture**: Designed for next-generation automotive applications

The Red Panda represents the pinnacle of panda technology, specifically designed for modern automotive applications requiring high-speed CAN-FD communication, intelligent harness management, and robust safety features.