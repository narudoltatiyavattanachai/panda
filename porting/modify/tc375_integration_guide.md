# Red Panda to TC375 Integration Guide

## Overview
This guide details how to integrate Red Panda firmware into the TC375 RTOS Gateway project structure.

## Directory Structure

### `/porting/modify/` - Reorganized for TC375 Integration

```
├── Apps/                    # Application layer components
├── Core/                    # Core firmware files
│   ├── main.c              # Main firmware entry point
│   ├── main_comms.h        # Communication protocol definitions
│   ├── main_declarations.h # Function declarations
│   ├── main_definitions.h  # Constants and definitions
│   ├── health.h           # Health monitoring
│   └── config.h           # Configuration
├── Drivers/                # Hardware drivers
│   ├── can_common.h        # CAN common functions
│   ├── can_common_declarations.h
│   ├── panda_fdcan.h       # FDCAN driver (to be adapted)
│   ├── panda_fdcan_declarations.h
│   ├── usb.h              # USB driver interface
│   └── usb_declarations.h
├── HAL/                    # Hardware Abstraction Layer
│   └── stm32h7_reference/  # STM32H7 reference implementation
├── Protocol/               # Communication protocols
│   ├── can_comms.h        # CAN communication protocol
│   ├── can_declarations.h  # CAN packet structures
│   ├── can.h              # CAN interface
│   └── comms_definitions.h # Communication definitions
├── Safety/                 # Safety and security
│   └── crypto/            # Cryptographic functions
├── Build/                  # Build system
│   ├── SConstruct         # Main build configuration
│   ├── SConscript         # Build script
│   └── board_SConscript   # Board-specific build
└── tc375_integration_guide.md
```

## Integration Steps

### 1. Core Firmware Integration

**Target Location in TC375 project**: `/Apps/panda/`

Copy the following files:
- `Core/main.c` → `Apps/panda/Panda_Main.c`
- `Core/main_*.h` → `Apps/panda/`
- `Core/health.h` → `Apps/panda/`
- `Core/config.h` → `Apps/panda/Panda_Config.h`

### 2. CAN Driver Integration

**Target Location in TC375 project**: `/Libraries/Panda_CAN/`

**Key Changes Required**:
- Replace FDCAN controller with MultiCAN controller
- Adapt `panda_fdcan.h` → `panda_multican.h`
- Integrate with existing TC375 CAN infrastructure in `Libraries/iLLD/TC37A/Tricore/Can/`

**Files to adapt**:
```c
// From: Drivers/panda_fdcan.h
// To:   Libraries/Panda_CAN/panda_multican.h
// Change: STM32H7 FDCAN → TC375 MultiCAN
```

### 3. USB Driver Integration

**Target Location in TC375 project**: `/Libraries/Panda_USB/`

**Key Changes Required**:
- Replace STM32H7 USB OTG with TC375 USB controller
- No direct USB support in current TC375 project - needs implementation

**Alternative**: Use Ethernet + TCP/IP bridge for PC communication

### 4. Protocol Integration

**Target Location in TC375 project**: `/Apps/panda/Protocol/`

**Files to copy**:
- `Protocol/can_*.h` → `Apps/panda/Protocol/`
- `Protocol/comms_definitions.h` → `Apps/panda/Protocol/`

**Integration points**:
- Integrate with existing FreeRTOS task system
- Use existing inter-CPU communication for CAN processing

### 5. Safety System Integration

**Target Location in TC375 project**: `/Apps/panda/Safety/`

**Files to copy**:
- `Safety/crypto/` → `Apps/panda/Safety/`

**Integration**:
- Use existing CPU isolation for safety-critical functions
- Integrate with FreeRTOS task priorities

## TC375 Architecture Mapping

### Red Panda → TC375 Mapping

| Red Panda Component | TC375 Equivalent | Notes |
|---------------------|------------------|-------|
| STM32H7 Main CPU | TC375 CPU0 (FreeRTOS) | Main application processing |
| CAN Controller | MultiCAN Controller | Hardware-level change required |
| USB Controller | Ethernet + TCP/IP | Protocol bridge needed |
| Safety System | CPU1/CPU2 Isolation | Use bare-metal CPUs for safety |
| Interrupts | SRC (Service Request Controller) | Different interrupt architecture |
| Flash/RAM | TC375 Memory Map | Different memory layout |

### Multi-CPU Architecture

```
CPU0 (FreeRTOS):
├── Main Panda application
├── USB/Ethernet protocol bridge
├── Safety coordination
└── Host communication

CPU1 (Bare-metal):
├── CAN message processing
├── Real-time safety checks
└── Hardware monitoring

CPU2 (Bare-metal):
├── Backup safety system
├── Watchdog management
└── Error handling
```

## Implementation Priority

### Phase 1: Core Integration
1. Copy core firmware files
2. Adapt main.c for FreeRTOS
3. Create basic CAN message handling

### Phase 2: CAN Driver
1. Implement MultiCAN driver
2. Port CAN protocol handling
3. Integrate with safety system

### Phase 3: Communication
1. Implement Ethernet bridge
2. Port USB protocol over TCP/IP
3. Add protocol compatibility layer

### Phase 4: Safety System
1. Implement multi-CPU safety
2. Port cryptographic functions
3. Add safety validation

## File Mapping for TC375

### Core Files
```bash
# Copy to TC375 project
cp Core/main.c ~/Winsurf/tc375_rtos_gw/Apps/panda/Panda_Main.c
cp Core/main_comms.h ~/Winsurf/tc375_rtos_gw/Apps/panda/
cp Core/config.h ~/Winsurf/tc375_rtos_gw/Apps/panda/Panda_Config.h
```

### CAN Driver Files
```bash
# Create CAN driver directory
mkdir -p ~/Winsurf/tc375_rtos_gw/Libraries/Panda_CAN/
cp Drivers/panda_fdcan.h ~/Winsurf/tc375_rtos_gw/Libraries/Panda_CAN/panda_multican.h
cp Drivers/can_common.h ~/Winsurf/tc375_rtos_gw/Libraries/Panda_CAN/
```

### Protocol Files
```bash
# Create protocol directory
mkdir -p ~/Winsurf/tc375_rtos_gw/Apps/panda/Protocol/
cp Protocol/*.h ~/Winsurf/tc375_rtos_gw/Apps/panda/Protocol/
```

## Next Steps

1. **Review TC375 existing CAN implementation**
2. **Adapt FDCAN driver to MultiCAN**
3. **Implement Ethernet bridge for PC communication**
4. **Integrate with FreeRTOS task system**
5. **Test CAN message handling**
6. **Implement safety validation**

## Compatibility Notes

- **CAN Protocol**: Should remain compatible at packet level
- **USB Protocol**: Needs Ethernet bridge implementation
- **Safety System**: Enhanced with multi-CPU architecture
- **Build System**: Replace SCons with TC375 build system