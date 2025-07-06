# Red Panda USB Protocol Specification

## Overview
The Red Panda uses a custom USB protocol to bridge communication between computers and vehicle CAN networks. This document details the complete protocol specification including packet formats, endpoints, and control commands.

## USB Device Configuration

### Device Descriptor
```
┌─────────────────────────────────────────────────────────────────┐
│                    USB DEVICE DESCRIPTOR                        │
├─────────────────────┬───────────────────────────────────────────┤
│ Field               │ Value                                     │
├─────────────────────┼───────────────────────────────────────────┤
│ USB Version         │ 2.1 (0x0210)                            │
│ Device Class        │ 0xFF (Vendor Specific)                   │
│ Device Subclass     │ 0xFF (Vendor Specific)                   │
│ Device Protocol     │ 0xFF (Vendor Specific)                   │
│ Max Packet Size     │ 64 bytes (0x40)                         │
│ Vendor ID (VID)     │ 0x3801 (comma.ai)                       │
│ Product ID (PID)    │ 0xDDCC (app) / 0xDDEE (bootloader)      │
│ Device Version      │ Firmware dependent                       │
│ Manufacturer String │ "comma.ai"                               │
│ Product String      │ "panda"                                  │
│ Serial Number       │ Derived from STM32 chip UID             │
└─────────────────────┴───────────────────────────────────────────┘
```

### Configuration Descriptor
```
┌─────────────────────────────────────────────────────────────────┐
│                  USB CONFIGURATION DESCRIPTOR                   │
├─────────────────────┬───────────────────────────────────────────┤
│ Field               │ Value                                     │
├─────────────────────┼───────────────────────────────────────────┤
│ Configuration Value │ 1                                         │
│ String Descriptor   │ "Panda Configuration"                     │
│ Attributes          │ 0x80 (Bus Powered)                       │
│ Max Power           │ 500mA (0xFA)                             │
│ Interface Count     │ 1                                         │
│ Endpoint Count      │ 3 (plus control EP0)                     │
└─────────────────────┴───────────────────────────────────────────┘
```

## USB Endpoint Configuration

### Endpoint Map
```
┌─────────────────────────────────────────────────────────────────┐
│                        USB ENDPOINTS                            │
├────────────┬─────────────┬──────────────────┬─────────────────────┤
│ Endpoint   │ Address     │ Type             │ Purpose             │
├────────────┼─────────────┼──────────────────┼─────────────────────┤
│ EP0        │ 0x00        │ Control          │ Device setup &      │
│            │             │ (Bidirectional)  │ control commands    │
├────────────┼─────────────┼──────────────────┼─────────────────────┤
│ EP1        │ 0x81        │ Bulk IN          │ CAN messages        │
│            │             │ (Device→Host)    │ from vehicle        │
├────────────┼─────────────┼──────────────────┼─────────────────────┤
│ EP2        │ 0x02        │ Bulk OUT         │ Serial/UART data    │
│            │             │ (Host→Device)    │ forwarding          │
├────────────┼─────────────┼──────────────────┼─────────────────────┤
│ EP3        │ 0x03        │ Bulk OUT         │ CAN messages        │
│            │             │ (Host→Device)    │ to vehicle          │
└────────────┴─────────────┴──────────────────┴─────────────────────┘
```

### Endpoint Properties
```
┌─────────────────────────────────────────────────────────────────┐
│                    ENDPOINT PROPERTIES                          │
├────────────┬─────────────┬─────────────────┬─────────────────────┤
│ Endpoint   │ Max Packet  │ Polling         │ Buffer Size         │
│            │ Size        │ Interval        │                     │
├────────────┼─────────────┼─────────────────┼─────────────────────┤
│ EP0        │ 64 bytes    │ N/A             │ 64 bytes            │
├────────────┼─────────────┼─────────────────┼─────────────────────┤
│ EP1 (IN)   │ 64 bytes    │ N/A (on-demand) │ 16384 bytes (host)  │
├────────────┼─────────────┼─────────────────┼─────────────────────┤
│ EP2 (OUT)  │ 64 bytes    │ N/A             │ 256 bytes           │
├────────────┼─────────────┼─────────────────┼─────────────────────┤
│ EP3 (OUT)  │ 64 bytes    │ N/A             │ 256 bytes           │
└────────────┴─────────────┴─────────────────┴─────────────────────┘
```

## CAN Packet Format

### Packet Structure
```
┌─────────────────────────────────────────────────────────────────┐
│                     CAN PACKET STRUCTURE                        │
│                   (Over USB Transfer)                           │
├─────────────────────┬───────────────────────────────────────────┤
│ Offset   │ Size     │ Field Description                         │
├─────────────────────┼───────────────────────────────────────────┤
│ 0        │ 1 byte   │ Header Byte 0                            │
│          │          │ ┌─ Bit 7-4: Data Length Code (DLC)       │
│          │          │ ├─ Bit 3-1: CAN Bus Number (0-2)         │
│          │          │ └─ Bit 0:   CAN-FD Flag                  │
├─────────────────────┼───────────────────────────────────────────┤
│ 1        │ 1 byte   │ Header Byte 1                            │
│          │          │ ┌─ Bit 7-2: Address bits [5:0]           │
│          │          │ ├─ Bit 1:   Returned Flag                │
│          │          │ └─ Bit 0:   Rejected Flag                │
├─────────────────────┼───────────────────────────────────────────┤
│ 2        │ 1 byte   │ Header Byte 2 - Address bits [13:6]      │
├─────────────────────┼───────────────────────────────────────────┤
│ 3        │ 1 byte   │ Header Byte 3 - Address bits [21:14]     │
├─────────────────────┼───────────────────────────────────────────┤
│ 4        │ 1 byte   │ Header Byte 4 - Address bits [28:22]     │
│          │          │ + Extended ID Flag (bit 2)               │
├─────────────────────┼───────────────────────────────────────────┤
│ 5        │ 1 byte   │ Checksum (XOR of header + data)          │
├─────────────────────┼───────────────────────────────────────────┤
│ 6        │ 0-64     │ CAN Data Payload                         │
│          │ bytes    │ (Length determined by DLC)               │
└─────────────────────┴───────────────────────────────────────────┘
```

### Header Byte Details
```
┌─────────────────────────────────────────────────────────────────┐
│                     HEADER BYTE 0 (Offset 0)                    │
├─────────────────────┬───────────────────────────────────────────┤
│ Bit   │ 7  6  5  4  │ 3  2  1     │ 0                          │
├─────────────────────┼─────────────┼────────────────────────────┤
│ Field │ Data Length │ CAN Bus     │ CAN-FD                     │
│       │ Code (DLC)  │ Number      │ Flag                       │
├─────────────────────┼─────────────┼────────────────────────────┤
│ Range │ 0-15        │ 0-7         │ 0/1                        │
└─────────────────────┴─────────────┴────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                     HEADER BYTE 1 (Offset 1)                    │
├─────────────────────┬───────────────────────────────────────────┤
│ Bit   │ 7  6  5  4  3  2  │ 1        │ 0                      │
├─────────────────────┼──────────────┼──────────────────────────┤
│ Field │ Address[5:0]     │ Returned │ Rejected               │
│       │ (Low 6 bits)     │ Flag     │ Flag                   │
├─────────────────────┼──────────────┼──────────────────────────┤
│ Desc  │ CAN ID bits      │ Echo/    │ Safety                 │
│       │                  │ Loopback │ Blocked                │
└─────────────────────┴──────────────┴──────────────────────────┘
```

### Data Length Code (DLC) Mapping
```
┌─────────────────────────────────────────────────────────────────┐
│                        DLC TO LENGTH                            │
├─────────────────────┬───────────────────────────────────────────┤
│ DLC Value (4 bits)  │ Data Length (bytes)                      │
├─────────────────────┼───────────────────────────────────────────┤
│ 0                   │ 0                                         │
│ 1                   │ 1                                         │
│ 2                   │ 2                                         │
│ 3                   │ 3                                         │
│ 4                   │ 4                                         │
│ 5                   │ 5                                         │
│ 6                   │ 6                                         │
│ 7                   │ 7                                         │
│ 8                   │ 8                                         │
│ 9                   │ 12 (CAN-FD only)                         │
│ 10                  │ 16 (CAN-FD only)                         │
│ 11                  │ 20 (CAN-FD only)                         │
│ 12                  │ 24 (CAN-FD only)                         │
│ 13                  │ 32 (CAN-FD only)                         │
│ 14                  │ 48 (CAN-FD only)                         │
│ 15                  │ 64 (CAN-FD only)                         │
└─────────────────────┴───────────────────────────────────────────┘
```

## Control Commands (EP0)

### Control Request Format
```
┌─────────────────────────────────────────────────────────────────┐
│                    CONTROL REQUEST STRUCTURE                    │
├─────────────────────┬───────────────────────────────────────────┤
│ Field               │ Description                               │
├─────────────────────┼───────────────────────────────────────────┤
│ bmRequestType       │ 0x40 (Vendor, Device, Host→Device)       │
│ bRequest            │ Command ID (see command table)           │
│ wValue              │ Parameter 1 (16-bit)                     │
│ wIndex              │ Parameter 2 (16-bit)                     │
│ wLength             │ Data length for data phase               │
│ Data                │ Optional data payload                     │
└─────────────────────┴───────────────────────────────────────────┘
```

### Command Table
```
┌─────────────────────────────────────────────────────────────────┐
│                       CONTROL COMMANDS                          │
├─────────┬─────────────────────────────────────────────────────────┤
│ Command │ Description                                           │
│ ID      │                                                       │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xc0    │ Reset Communication                                   │
│         │ ├─ Clears all buffers and resets state               │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xc1    │ Enter DFU Mode                                        │
│         │ ├─ Switches to bootloader for firmware update        │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xc2    │ Get CAN Health Stats                                  │
│         │ ├─ Returns CAN bus error counters                    │
│         │ └─ Parameter: Bus number (0-2)                       │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xd0    │ Set CAN Loopback Mode                                 │
│         │ ├─ Enable/disable CAN loopback testing               │
│         │ └─ Parameter: Enable (1) / Disable (0)               │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xd1    │ Set CAN Silent Mode                                   │
│         │ ├─ Enable/disable CAN silent monitoring              │
│         │ └─ Parameter: Enable (1) / Disable (0)               │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xd2    │ Get Health Packet                                     │
│         │ ├─ Returns device health and status info             │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xd6    │ Set CAN Speed                                         │
│         │ ├─ Configure CAN bus bitrate                         │
│         │ ├─ Parameter 1: Bus number (0-2)                     │
│         │ └─ Parameter 2: Speed (0=125k, 1=250k, 2=500k, etc) │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xd8    │ Get CAN Speed                                         │
│         │ ├─ Query current CAN bus bitrate                     │
│         │ └─ Parameter: Bus number (0-2)                       │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xda    │ Set UART Bitrate                                      │
│         │ ├─ Configure UART speed for serial forwarding        │
│         │ ├─ Parameter 1: UART number                          │
│         │ └─ Parameter 2: Bitrate value                        │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xdc    │ Set Safety Mode                                       │
│         │ ├─ Configure vehicle safety model                    │
│         │ ├─ Parameter 1: Safety mode ID                       │
│         │ └─ Parameter 2: Safety parameter                     │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xdd    │ Get Safety Mode                                       │
│         │ ├─ Query current safety configuration                │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xde    │ Set CAN FD Non-ISO Mode                              │
│         │ ├─ Enable/disable non-ISO CAN-FD mode                │
│         │ ├─ Parameter 1: Bus number (0-2)                     │
│         │ └─ Parameter 2: Enable (1) / Disable (0)             │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xe0    │ Get Version                                           │
│         │ ├─ Returns firmware version string                   │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xe1    │ Get Device ID                                         │
│         │ ├─ Returns unique device identifier                  │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xe6    │ Set Heartbeat Disabled                                │
│         │ ├─ Disable periodic heartbeat requirement            │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xf0    │ Get RTC Time                                          │
│         │ ├─ Returns real-time clock value                     │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xf1    │ Clear CAN Ring Buffer                                 │
│         │ ├─ Flush all pending CAN messages                    │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xf2    │ Send Heartbeat                                        │
│         │ ├─ Send periodic heartbeat to maintain connection    │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xf3    │ Get Harness Status                                    │
│         │ ├─ Returns harness connection and orientation        │
│         │ └─ No parameters                                      │
├─────────┼─────────────────────────────────────────────────────────┤
│ 0xf4    │ Get Interrupt Load                                    │
│         │ ├─ Returns system interrupt statistics               │
│         │ └─ No parameters                                      │
└─────────┴─────────────────────────────────────────────────────────┘
```

## Data Transfer Protocol

### CAN Message Transmission (EP3 OUT)
```
┌─────────────────────────────────────────────────────────────────┐
│                   CAN TX PROTOCOL FLOW                          │
└─────────────────────────────────────────────────────────────────┘

1. Host Application (Python)
   │
   ├─ pack_can_buffer() - Format CAN messages into USB packets
   │  ├─ Add 6-byte headers to each message
   │  ├─ Calculate checksums
   │  └─ Group into 64-byte USB transfers
   │
   ├─ USB Bulk Transfer (EP3 OUT)
   │  ├─ Transfer chunked data to device
   │  └─ Handle partial transfers
   │
   └─ Device Processing
      ├─ comms_can_write() - Parse USB data
      ├─ Safety validation - Check safety hooks
      ├─ CAN hardware - Send via FDCAN controller
      └─ Optional echo - Return via EP1 if enabled
```

### CAN Message Reception (EP1 IN)
```
┌─────────────────────────────────────────────────────────────────┐
│                   CAN RX PROTOCOL FLOW                          │
└─────────────────────────────────────────────────────────────────┘

1. Vehicle CAN Bus
   │
   ├─ FDCAN Hardware - Receive CAN frames
   │  ├─ Hardware filtering (if configured)
   │  └─ Interrupt-driven reception
   │
   ├─ Device Processing
   │  ├─ can_rx_hook() - Process received message
   │  ├─ Add to RX queue - Buffer for USB transfer
   │  └─ Pack into USB format - Add headers/checksums
   │
   └─ USB Bulk Transfer (EP1 IN)
      ├─ comms_can_read() - Prepare USB data
      ├─ Bulk transfer to host
      └─ Handle overflow buffering

2. Host Application (Python)
   │
   ├─ USB Bulk Read (EP1 IN)
   │  ├─ Read up to 16384 bytes per transfer
   │  └─ Handle partial packet buffering
   │
   └─ unpack_can_buffer() - Parse USB data
      ├─ Extract CAN messages from USB packets
      ├─ Validate checksums
      └─ Return structured CAN data
```

## Health and Status Monitoring

### Health Packet Structure
```
┌─────────────────────────────────────────────────────────────────┐
│                     HEALTH PACKET FORMAT                        │
├─────────────────────┬───────────────────────────────────────────┤
│ Offset │ Size       │ Field Description                         │
├─────────────────────┼───────────────────────────────────────────┤
│ 0      │ 4 bytes    │ Voltage (12V supply, in mV)              │
│ 4      │ 4 bytes    │ Current (supply current, in mA)           │
│ 8      │ 1 byte     │ Harness Status (0=NC, 1=Normal, 2=Flip)  │
│ 9      │ 1 byte     │ Ignition State (0=Off, 1=On)             │
│ 10     │ 1 byte     │ Safety Mode (current safety config)      │
│ 11     │ 1 byte     │ Safety Parameter                          │
│ 12     │ 4 bytes    │ Safety TX Blocked Count                   │
│ 16     │ 4 bytes    │ Safety TX Allowed Count                   │
│ 20     │ 4 bytes    │ Safety RX Invalid Count                   │
│ 24     │ 4 bytes    │ Safety RX Valid Count                     │
│ 28     │ 4 bytes    │ Heartbeat Lost Count                      │
│ 32     │ 4 bytes    │ Uptime (seconds since boot)               │
│ 36     │ 2 bytes    │ SBU1 Voltage (ADC reading)                │
│ 38     │ 2 bytes    │ SBU2 Voltage (ADC reading)                │
│ 40     │ 4 bytes    │ Interrupt Load (% CPU time)               │
│ 44     │ 4 bytes    │ CAN Error Count (bus 0)                   │
│ 48     │ 4 bytes    │ CAN Error Count (bus 1)                   │
│ 52     │ 4 bytes    │ CAN Error Count (bus 2)                   │
└─────────────────────┴───────────────────────────────────────────┘
```

### CAN Health Packet Structure
```
┌─────────────────────────────────────────────────────────────────┐
│                   CAN HEALTH PACKET FORMAT                      │
├─────────────────────┬───────────────────────────────────────────┤
│ Offset │ Size       │ Field Description                         │
├─────────────────────┼───────────────────────────────────────────┤
│ 0      │ 4 bytes    │ TX Error Count                            │
│ 4      │ 4 bytes    │ RX Error Count                            │
│ 8      │ 4 bytes    │ TX Overflow Count                         │
│ 12     │ 4 bytes    │ RX Overflow Count                         │
│ 16     │ 4 bytes    │ Last Error Code                           │
│ 20     │ 1 byte     │ Bus State (0=Active, 1=Passive, 2=Off)   │
│ 21     │ 1 byte     │ Error Warning Flag                        │
│ 22     │ 1 byte     │ Error Passive Flag                        │
│ 23     │ 1 byte     │ Bus Off Flag                              │
│ 24     │ 4 bytes    │ Arbitration Lost Count                    │
│ 28     │ 4 bytes    │ CAN FD Messages Count                     │
│ 32     │ 4 bytes    │ CAN FD BRS Count                          │
│ 36     │ 4 bytes    │ CAN FD ESI Count                          │
└─────────────────────┴───────────────────────────────────────────┘
```

## Protocol Versions and Compatibility

### Version Management
```
┌─────────────────────────────────────────────────────────────────┐
│                    PROTOCOL VERSIONS                            │
├─────────────────────┬───────────────────────────────────────────┤
│ Version Type        │ Current Version │ Description             │
├─────────────────────┼─────────────────┼─────────────────────────┤
│ CAN_PACKET_VERSION  │ 4               │ CAN packet format       │
│ HEALTH_PACKET_VER   │ 16              │ Health data structure   │
│ CAN_HEALTH_PACKET   │ 5               │ CAN health statistics   │
│ USBPROTOCOL_VERSION │ 1               │ USB protocol version    │
└─────────────────────┴─────────────────┴─────────────────────────┘
```

### Error Handling
```
┌─────────────────────────────────────────────────────────────────┐
│                       ERROR HANDLING                            │
├─────────────────────┬───────────────────────────────────────────┤
│ Error Type          │ Handling Method                           │
├─────────────────────┼───────────────────────────────────────────┤
│ Version Mismatch    │ RuntimeError with version info            │
│ Checksum Failure    │ Packet dropped, assertion error           │
│ USB Transfer Error  │ Timeout, retry mechanism                  │
│ Safety Rejection    │ Message marked as rejected (bit flag)     │
│ Buffer Overflow     │ Overflow counter increment                │
│ CAN Bus Error       │ Error counter increment, health reporting │
└─────────────────────┴───────────────────────────────────────────┘
```

## Performance Characteristics

### Transfer Limits
```
┌─────────────────────────────────────────────────────────────────┐
│                    PERFORMANCE LIMITS                           │
├─────────────────────┬───────────────────────────────────────────┤
│ Parameter           │ Value                                     │
├─────────────────────┼───────────────────────────────────────────┤
│ USB Speed           │ 12 Mbps (Full Speed)                     │
│ Max USB Packet      │ 64 bytes                                  │
│ Max Bulk Transfer   │ 16384 bytes (host side)                  │
│ Max CAN Msgs/TX     │ 51 messages per USB transfer             │
│ CAN TX Queue        │ 256 messages (hardware dependent)        │
│ CAN RX Queue        │ 1024 messages (hardware dependent)       │
│ Heartbeat Timeout   │ 5 seconds (configurable)                 │
│ Control Timeout     │ 1 second (default)                       │
│ CAN Send Timeout    │ 5 seconds (default)                      │
└─────────────────────┴───────────────────────────────────────────┘
```

### Latency Characteristics
```
┌─────────────────────────────────────────────────────────────────┐
│                      LATENCY ANALYSIS                           │
├─────────────────────┬───────────────────────────────────────────┤
│ Path                │ Typical Latency                           │
├─────────────────────┼───────────────────────────────────────────┤
│ USB Control Command │ 1-5 ms                                   │
│ CAN TX (Host→Bus)   │ 2-10 ms                                  │
│ CAN RX (Bus→Host)   │ 1-20 ms (depends on polling)            │
│ Safety Validation   │ < 1 ms                                   │
│ USB Bulk Transfer   │ 1-2 ms per 64-byte packet               │
│ End-to-End Latency  │ 5-50 ms (application dependent)         │
└─────────────────────┴───────────────────────────────────────────┘
```

## STM32 Firmware CAN Intelligence

### **Evidence: Deep CAN Protocol Understanding vs Raw Data Forwarding**

While DBC files handle signal definitions on the PC side, the STM32 firmware demonstrates **substantial CAN protocol understanding** that goes far beyond simple data forwarding:

#### **1. Hardware-Level CAN Protocol Management**
```c
// STM32 understands and manages CAN protocol error states
can_health[can_number].bus_off = ((psr_reg & FDCAN_PSR_BO) >> FDCAN_PSR_BO_Pos);
can_health[can_number].error_warning = ((psr_reg & FDCAN_PSR_EW) >> FDCAN_PSR_EW_Pos);
can_health[can_number].error_passive = ((psr_reg & FDCAN_PSR_EP) >> FDCAN_PSR_EP_Pos);
can_health[can_number].last_error = ((psr_reg & FDCAN_PSR_LEC) >> FDCAN_PSR_LEC_Pos);
```

#### **2. CAN Frame Structure Intelligence**
```c
typedef struct {
  unsigned char fd : 1;           // CAN-FD frame detection
  unsigned char bus : 3;          // Intelligent bus routing
  unsigned char data_len_code : 4; // DLC protocol understanding
  unsigned char rejected : 1;     // Safety system integration
  unsigned char returned : 1;     // Echo/loopback management
  unsigned char extended : 1;     // 11-bit vs 29-bit CAN ID handling
  unsigned int addr : 29;         // CAN identifier parsing
  unsigned char checksum;         // Integrity validation
  unsigned char data[64];         // Variable length payload support
} CANPacket_t;
```

#### **3. DLC-to-Length Protocol Mapping**
```c
// STM32 firmware understands CAN Data Length Code mapping
static const unsigned char dlc_to_len[] = {
  0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,     // Standard CAN
  12U, 16U, 20U, 24U, 32U, 48U, 64U       // CAN-FD extensions
};
```

#### **4. Vehicle-Specific CAN Message Interpretation**
```c
// GM-specific ignition detection (Hard-coded vehicle knowledge)
if ((addr == 0x1F1) && (len == 8)) {
  // SystemPowerMode (2=Run, 3=Crank Request)
  ignition_can = (GET_BYTE(to_push, 0) & 0x2U) != 0U;
}

// Tesla Model 3/Y specific parsing with counter validation
if ((addr == 0x221) && (len == 8)) {
  int counter = GET_BYTE(to_push, 6) >> 4;
  if ((counter == ((prev_counter_tesla + 1) % 16)) && (prev_counter_tesla != -1)) {
    // VCFRONT_vehiclePowerState semantic understanding
    int power_state = (GET_BYTE(to_push, 0) >> 5U) & 0x3U;
    ignition_can = power_state == 0x3;  // VEHICLE_POWER_STATE_DRIVE=3
  }
}
```

#### **5. CAN-FD Protocol Auto-Detection**
```c
// STM32 automatically detects and enables CAN-FD features
if (!(bus_config[can_number].canfd_enabled) && (canfd_frame)) {
  bus_config[can_number].canfd_enabled = true;
}
if (!(bus_config[can_number].brs_enabled) && (brs_frame)) {
  bus_config[can_number].brs_enabled = true;
}
```

#### **6. Intelligent Safety System Integration**
```c
// Safety system validates CAN messages before transmission
if (skip_tx_hook || safety_tx_hook(to_push) != 0) {
  // Message allowed - understands content for validation
  tx_buffer_overflow += can_push(can_queues[bus_number], to_push) ? 0U : 1U;
} else {
  // Message blocked by safety system based on CAN content analysis
  safety_tx_blocked += 1U;
  to_push->rejected = 1U;
}

// Intelligent message forwarding with safety-based routing
int bus_fwd_num = safety_fwd_hook(bus_number, to_push.addr);
```

### **Key Distinction: DBC vs Firmware Intelligence**

| **DBC Files (PC Side)** | **STM32 Firmware Intelligence** |
|-------------------------|----------------------------------|
| Signal definitions & scaling | CAN protocol mechanics & frame structure |
| Message names & descriptions | Hardware-level error state management |
| Unit conversions | Vehicle-specific message parsing |
| Application-level semantics | Real-time safety validation |
| Human-readable mapping | Protocol state management (CAN-FD, bus health) |
| Development convenience | Safety-critical message routing |

### **Conclusion: Smart CAN Interface**

The STM32 firmware is **NOT** a "dumb" data bridge but rather a **sophisticated CAN interface** with:

1. **Deep CAN Protocol Knowledge**: Frame structure, error states, DLC mapping, CAN-FD support
2. **Vehicle-Specific Intelligence**: Hard-coded knowledge of manufacturer-specific messages
3. **Real-time Safety Integration**: CAN-aware safety hooks that validate message content
4. **Protocol State Management**: Automatic CAN-FD detection, bus health monitoring
5. **Intelligent Routing**: Safety-based message forwarding between buses

This intelligence is essential for real-time safety-critical operations that cannot rely on PC-side processing latency.

## CAN Knowledge Sources: Hybrid Architecture

### **How STM32 Gets CAN Protocol Knowledge**

The STM32 firmware uses a **hybrid approach** combining hard-coded knowledge, runtime-downloaded rules, and PC-side processing:

```
┌─────────────────────────────────────────────────────────────────┐
│                    CAN KNOWLEDGE SOURCES                        │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│ 1. HARD-CODED in STM32 Firmware:                              │
│    ├─ Basic CAN protocol (frame structure, DLC, errors)       │
│    ├─ Vehicle ignition detection (GM, Tesla, Rivian)          │
│    ├─ CAN-FD protocol mechanics                               │
│    └─ Hardware-level error management                         │
│                                                                 │
│ 2. DOWNLOADED from PC via USB:                                │
│    ├─ Safety models (opendbc/safety/)                         │
│    ├─ Vehicle-specific safety rules                           │
│    ├─ Message filtering/blocking logic                        │
│    └─ Safety hook functions                                   │
│                                                                 │
│ 3. DBC Files (PC Only):                                       │
│    ├─ Signal definitions & scaling                            │
│    ├─ Message names & descriptions                            │
│    └─ Application-level semantics                             │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### **Knowledge Layer Details**

#### **1. Hard-Coded Knowledge (Compiled into STM32 Firmware)**
```c
// Basic CAN protocol understanding - BUILT-IN
static const unsigned char dlc_to_len[] = {
  0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,     // Standard CAN
  12U, 16U, 20U, 24U, 32U, 48U, 64U       // CAN-FD extensions
};

// Vehicle-specific ignition detection - HARD-CODED
void ignition_can_hook(CANPacket_t *to_push) {
  // GM exception - HARD-CODED vehicle knowledge
  if ((addr == 0x1F1) && (len == 8)) {
    // SystemPowerMode (2=Run, 3=Crank Request)
    ignition_can = (GET_BYTE(to_push, 0) & 0x2U) != 0U;
  }
  
  // Tesla Model 3/Y - HARD-CODED vehicle knowledge  
  if ((addr == 0x221) && (len == 8)) {
    // VCFRONT_vehiclePowerState - HARD-CODED bit parsing
    int power_state = (GET_BYTE(to_push, 0) >> 5U) & 0x3U;
    ignition_can = power_state == 0x3;  // VEHICLE_POWER_STATE_DRIVE=3
  }
}
```

#### **2. Runtime-Downloaded Knowledge (PC → STM32 via USB)**
```python
# PC side: Download safety model to STM32
p.set_safety_mode(CarParams.SafetyModel.toyota, param=0)
# USB Control Command 0xdc → STM32 loads Toyota-specific safety rules
```

```c
// STM32 side: Receive and apply safety mode
void set_safety_mode(uint16_t mode, uint16_t param) {
  int err = set_safety_hooks(mode_copy, param);  // Links to opendbc safety
  // Available modes: SAFETY_TOYOTA, SAFETY_HONDA, SAFETY_GM, etc.
}
```

#### **3. PC-Only Knowledge (DBC Files)**
```python
# PC side: Signal definitions and scaling (NOT sent to STM32)
# Example: Engine RPM signal scaling, unit conversion, human names
from opendbc import dbc
```

### **Message Processing Flow**

#### **Real-Time Processing (STM32)**
```c
// Each CAN message processed using HYBRID knowledge:
void process_can_message(CANPacket_t *msg) {
  // 1. HARD-CODED: Basic protocol validation
  if (!validate_can_frame(msg)) return;
  
  // 2. HARD-CODED: Vehicle-specific processing
  ignition_can_hook(msg);  // GM/Tesla/Rivian ignition detection
  
  // 3. DOWNLOADED: Safety rule validation  
  if (!safety_tx_hook(msg)) {
    msg->rejected = 1;  // Block unsafe message
    return;
  }
  
  // 4. DOWNLOADED: Intelligent routing
  int fwd_bus = safety_fwd_hook(bus, msg->addr);
}
```

#### **Development Processing (PC)**
```python
# PC side: Human-readable processing using DBC
def process_message_for_display(can_id, data):
  # Use DBC to decode signal names, units, scaling
  parsed = dbc.decode_message(can_id, data)
  # Result: {"EngineRPM": 2500, "VehicleSpeed": 65.5}
```

### **Why This Hybrid Architecture?**

#### **Hard-Coded Advantages:**
- **Real-time Performance**: No USB communication delay
- **Safety Critical**: Works even if PC communication fails
- **Universal**: Core CAN protocol works across all vehicles
- **Reliability**: Cannot be corrupted by PC-side issues

#### **Downloaded Knowledge Advantages:**
- **Updateable**: Safety rules can be updated without firmware changes
- **Vehicle-Specific**: Different safety models for different manufacturers
- **Comprehensive**: Complex safety logic from opendbc project
- **Modular**: Easy to add new vehicle support

#### **PC-Only Advantages:**
- **Storage Efficient**: DBC files too large for STM32 memory
- **Development Friendly**: Human-readable signal names and units
- **Non-Critical**: Application convenience, not safety-critical
- **Flexible**: Easy modification without embedded constraints

### **Example Complete Flow:**
```
1. STM32 HARD-CODED: "Received CAN ID 0x1F1 with 8 bytes, valid frame"
2. STM32 HARD-CODED: "This matches GM ignition detection pattern"  
3. STM32 DOWNLOADED: "Apply Toyota safety rules - message allowed"
4. STM32 HARD-CODED: "Forward to USB with checksum validation"
5. PC DBC PROCESSING: "Decode as 'SystemPowerMode' = 'Drive'"
```

### **Safety Model Download Process:**
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ PC Application  │───▶│ USB Control 0xdc│───▶│ STM32 Firmware  │
│ CarParams.      │    │ mode=toyota     │    │ set_safety_     │
│ SafetyModel.    │    │ param=0         │    │ hooks()         │
│ toyota          │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                              │
                              ▼
                    ┌─────────────────────┐
                    │ opendbc/safety/     │
                    │ safety_toyota.h     │
                    │ (Compiled into      │
                    │  firmware)          │
                    └─────────────────────┘
```

## Summary

The Red Panda USB protocol provides a comprehensive bridge between USB and CAN domains:

1. **USB Layer**: Standard USB 2.0 with vendor-specific device class
2. **Endpoints**: 4 endpoints for control, CAN TX/RX, and serial forwarding
3. **Packet Format**: 6-byte header + payload with checksum validation
4. **Control Interface**: Rich command set for configuration and monitoring
5. **Data Flow**: Bidirectional CAN message forwarding with safety validation
6. **Health Monitoring**: Comprehensive system status and error reporting
7. **Version Management**: Protocol versioning for compatibility assurance
8. **STM32 Intelligence**: Deep CAN protocol understanding with vehicle-specific knowledge

The protocol efficiently handles the impedance mismatch between high-speed USB (12 Mbps) and automotive CAN networks (125 kbps - 5 Mbps), providing real-time vehicle communication capabilities while maintaining safety and reliability standards through intelligent firmware that understands both CAN protocol mechanics and vehicle-specific communication patterns.