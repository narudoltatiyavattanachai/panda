# Red Panda to TC375 TriCore Porting

## Overview

This directory contains the Red Panda firmware files reorganized and adapted for integration with the **TC375 RTOS Gateway** project. The original Red Panda was designed for STM32H7 ARM Cortex-M7 and needs to be ported to TC375 TriCore architecture.

## Project Structure

```
/porting/modify/
├── Apps/                           # Application Layer
│   └── panda_app_integration.h     # FreeRTOS integration definitions
├── Build/                          # Build System (reference)
│   ├── SConstruct                  # Original SCons build (reference)
│   ├── SConscript                  # Build scripts (reference)
│   └── board_SConscript           # Board-specific build (reference)
├── Core/                          # Core Firmware
│   ├── main.c                     # Main firmware entry point
│   ├── main_comms.h              # Communication protocol definitions
│   ├── main_declarations.h       # Function declarations
│   ├── main_definitions.h        # Constants and definitions
│   ├── config.h                  # System configuration
│   └── health.h                  # Health monitoring
├── Drivers/                       # Hardware Drivers
│   ├── can_common.h              # CAN common functions (portable)
│   ├── can_common_declarations.h # CAN declarations (portable)
│   ├── panda_fdcan.h            # Original FDCAN driver (reference)
│   ├── panda_fdcan_declarations.h# FDCAN declarations (reference)
│   ├── tc375_multican_adapter.h  # NEW: TC375 MultiCAN adapter
│   ├── usb.h                     # USB driver interface
│   └── usb_declarations.h        # USB declarations
├── HAL/                          # Hardware Abstraction Layer
│   └── stm32h7_reference/        # STM32H7 reference implementation
├── Protocol/                     # Communication Protocols
│   ├── can.h                     # CAN interface definitions
│   ├── can_comms.h              # CAN communication protocol
│   ├── can_declarations.h        # CAN packet structures
│   └── comms_definitions.h       # Communication definitions
├── Safety/                       # Safety & Security
│   └── crypto/                   # Cryptographic functions
│       ├── rsa.c, rsa.h         # RSA signature verification
│       ├── sha.c, sha.h         # SHA hashing
│       └── sign.py              # Signing tool
└── tc375_integration_guide.md   # Integration guide
```

## Key Adaptations for TC375

### 1. **MultiCAN Controller Adaptation**
- **File**: `Drivers/tc375_multican_adapter.h`
- **Purpose**: Provides compatibility layer between Red Panda's FDCAN API and TC375's MultiCAN controller
- **Key Features**:
  - API compatibility with original FDCAN driver
  - Support for CAN-FD and Bit Rate Switch (BRS)
  - Error handling and health monitoring
  - Message filtering and routing

### 2. **FreeRTOS Integration**
- **File**: `Apps/panda_app_integration.h`
- **Purpose**: Integrates Red Panda firmware as FreeRTOS tasks
- **Key Features**:
  - Multi-task architecture (Main, CAN RX/TX, USB, Safety)
  - Inter-CPU communication for TC375's multi-core architecture
  - Ethernet bridge for USB protocol (TC375 lacks USB)
  - Thread-safe operations with mutexes and queues

### 3. **Multi-CPU Architecture**
- **CPU0**: FreeRTOS main application, USB/Ethernet bridge, coordination
- **CPU1**: Real-time CAN message processing, safety validation
- **CPU2**: Backup safety monitoring, watchdog management

## Integration Steps

### Phase 1: Basic Integration
1. Copy core files to TC375 project:
   ```bash
   cp Core/main.c ~/Winsurf/tc375_rtos_gw/Apps/panda/Panda_Main.c
   cp Core/*.h ~/Winsurf/tc375_rtos_gw/Apps/panda/
   ```

2. Create Panda application directory:
   ```bash
   mkdir -p ~/Winsurf/tc375_rtos_gw/Apps/panda/{Core,Drivers,Protocol,Safety}
   ```

### Phase 2: CAN Driver Integration
1. Implement MultiCAN driver:
   ```bash
   cp Drivers/tc375_multican_adapter.h ~/Winsurf/tc375_rtos_gw/Libraries/Panda_CAN/
   ```

2. Adapt to TC375 iLLD CAN API:
   - Use `IfxCan_Can.h` for CAN operations
   - Integrate with existing pin mappings
   - Configure CAN nodes for Red Panda compatibility

### Phase 3: Communication Bridge
1. Implement Ethernet bridge for USB protocol
2. Create TCP/IP server for host communication
3. Maintain protocol compatibility with original Red Panda

### Phase 4: Safety System
1. Implement multi-CPU safety validation
2. Port cryptographic functions
3. Setup watchdog and error handling

## Hardware Mapping

| Red Panda (STM32H7) | TC375 Equivalent | Notes |
|---------------------|------------------|-------|
| FDCAN Controller | MultiCAN Controller | Hardware-level adaptation required |
| USB OTG | Ethernet + TCP/IP | Protocol bridge needed |
| ARM Cortex-M7 | TriCore CPU0 (FreeRTOS) | Main application processing |
| Single CPU | Multi-CPU (CPU1/2) | Enhanced safety architecture |
| NVIC Interrupts | SRC Controller | Different interrupt handling |

## Key Files for TC375 Development

### Essential Files to Port:
1. **`Core/main.c`** - Main firmware logic
2. **`Protocol/can_declarations.h`** - CAN packet structures (portable)
3. **`Drivers/can_common.h`** - CAN functions (mostly portable)
4. **`Safety/crypto/`** - Security functions (portable)

### Files Requiring Major Changes:
1. **`Drivers/panda_fdcan.h`** → Needs complete rewrite for MultiCAN
2. **`Drivers/usb.h`** → Replace with Ethernet bridge
3. **`HAL/stm32h7_reference/`** → Replace with TC375 HAL

### New Files Created:
1. **`Drivers/tc375_multican_adapter.h`** - MultiCAN compatibility layer
2. **`Apps/panda_app_integration.h`** - FreeRTOS integration
3. **`tc375_integration_guide.md`** - Detailed integration guide

## Protocol Compatibility

- **CAN Protocol**: Fully compatible at packet level
- **USB Protocol**: Bridged over Ethernet/TCP-IP
- **Safety System**: Enhanced with multi-CPU validation
- **Cryptographic**: Fully portable (standard algorithms)

## Build System

Original Red Panda uses **SCons** build system (Python-based). TC375 project likely uses:
- **AURIX Development Studio** (Eclipse-based)
- **TASKING Compiler** or **GCC TriCore**
- **Makefile** or **CMake** build system

Build files in `Build/` directory are provided for reference only.

## Next Development Steps

1. **Study TC375 MultiCAN controller** documentation
2. **Implement `tc375_multican_adapter.c`** based on header
3. **Create FreeRTOS task structure** as defined in `panda_app_integration.h`
4. **Implement Ethernet bridge** for host communication
5. **Setup inter-CPU communication** for safety functions
6. **Port and test** core CAN functionality
7. **Integrate safety system** with multi-CPU architecture

## Testing Strategy

1. **Unit Testing**: Test individual drivers and functions
2. **Integration Testing**: Test CAN communication end-to-end
3. **Safety Testing**: Validate safety system with fault injection
4. **Protocol Testing**: Verify compatibility with Red Panda Python library
5. **Performance Testing**: Measure real-time performance on TC375

## Compatibility Notes

- **CAN messages**: Should work identically to original Red Panda
- **Host software**: Should work without modification (via Ethernet bridge)
- **Safety models**: Need adaptation for multi-CPU architecture
- **Performance**: Should be enhanced due to dedicated CAN processing core

This porting effort maintains Red Panda's core functionality while leveraging TC375's enhanced multi-core safety architecture.