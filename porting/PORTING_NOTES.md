# Red Panda to TC275 TriCore Porting Notes

## Architecture Overview
The Red Panda firmware is designed for STM32H7 (ARM Cortex-M7) and needs to be ported to TC275 (TriCore).

## Key Files for Porting

### Core Firmware
- `board/main.c` - Main firmware entry point with USB and CAN handling
- `board/main_comms.h` - Communication protocol definitions
- `board/main_declarations.h` - Function declarations
- `board/main_definitions.h` - Constants and definitions

### Hardware Abstraction Layer (HAL)
- `board/stm32h7/` - STM32H7 specific implementations (to be replaced with TC275)
  - `board.h` - Board configuration
  - `clock.h` - Clock configuration
  - `peripherals.h` - Peripheral definitions
  - `ll*.h` - Low-level drivers (USB, CAN, SPI, UART, etc.)

### CAN Driver System
- `board/drivers/fdcan.h` - CAN-FD driver for STM32H7
- `board/drivers/can_common.h` - Common CAN functions and packet handling
- `board/can_declarations.h` - CAN packet structures
- `board/can_comms.h` - CAN communication protocol

### USB Driver System
- `board/drivers/usb.h` - USB driver interface
- `board/stm32h7/llusb.h` - Low-level USB implementation

### Safety System
- Safety hooks are integrated into `board/main.c`
- Safety models are loaded from host via USB control commands
- Functions: `safety_tx_hook()`, `safety_rx_hook()`, `safety_fwd_hook()`

### Build System
- `SConstruct` - Main build configuration
- `SConscript` - Build script
- `board/SConscript` - Board-specific build rules

### Crypto System
- `crypto/` - RSA signature verification for firmware updates
- `certs/` - Public keys for firmware signing

## Key Porting Challenges

### 1. CAN Controller Differences
- STM32H7 uses FDCAN controller
- TC275 uses MultiCAN controller
- Need to rewrite `board/drivers/fdcan.h` → `board/drivers/multican.h`

### 2. USB Controller Differences
- STM32H7 uses USB OTG controller
- TC275 uses USB 2.0 controller
- Need to rewrite `board/stm32h7/llusb.h` → `board/tc275/llusb.h`

### 3. Interrupt System
- STM32H7 uses NVIC (Nested Vector Interrupt Controller)
- TC275 uses different interrupt architecture
- Need to adapt `board/stm32h7/interrupt_handlers.h`

### 4. Memory Map
- STM32H7 has specific memory layout (Flash, RAM, DTCM, ITCM)
- TC275 has different memory architecture
- Need to adapt `board/stm32h7/stm32h7x5_flash.ld` → `board/tc275/tc275_flash.ld`

### 5. Clock System
- STM32H7 has HSE/HSI/PLL clock tree
- TC275 has different clock architecture
- Need to rewrite `board/stm32h7/clock.h`

## Protocol Compatibility
- CAN packet format should remain compatible
- USB protocol should remain compatible
- Safety system interface should remain compatible

## Next Steps
1. Create `board/tc275/` directory structure
2. Implement TC275-specific HAL
3. Port CAN driver to MultiCAN
4. Port USB driver to TC275 USB controller
5. Adapt build system for TC275 toolchain
6. Test protocol compatibility