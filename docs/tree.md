# Panda Project Directory Structure

## Overview
The panda project is a CAN/CAN-FD interface device developed by comma.ai that serves as a bridge between computers and vehicle CAN networks. It runs on STM32F413 and STM32H725 microcontrollers and consists of embedded firmware (C) and a Python library for host communication.

## Directory Tree Structure

```
panda/
├── 📁 board/                    # STM32 Embedded Firmware (C)
│   ├── 📄 main.c               # Main firmware entry point
│   ├── 📄 bootstub.c           # Bootloader stub for firmware updates
│   ├── 📄 can_comms.h          # CAN communication protocol definitions
│   ├── 📄 main_comms.h         # Main communication interface definitions
│   ├── 📄 obj/                 # Build artifacts and object files
│   ├── 📁 boards/              # Board-specific configurations
│   │   ├── 📄 black.h          # Black panda hardware configuration
│   │   ├── 📄 red.h            # Red panda hardware configuration
│   │   ├── 📄 white.h          # White panda hardware configuration
│   │   ├── 📄 tres.h           # Tres panda hardware configuration
│   │   ├── 📄 cuatro.h         # Cuatro panda hardware configuration
│   │   ├── 📄 dos.h            # Dos panda hardware configuration
│   │   └── 📄 board_declarations.h # Common board interface declarations
│   ├── 📁 drivers/             # Hardware Abstraction Layer (HAL)
│   │   ├── 📄 can_common.h     # Common CAN driver functionality
│   │   ├── 📄 bxcan.h          # Basic Extended CAN driver (STM32F4)
│   │   ├── 📄 fdcan.h          # CAN-FD driver (STM32H7)
│   │   ├── 📄 usb.h            # USB OTG driver
│   │   ├── 📄 spi.h            # SPI communication driver
│   │   ├── 📄 uart.h           # UART driver for debugging
│   │   ├── 📄 gpio.h           # GPIO control
│   │   ├── 📄 led.h            # LED control
│   │   ├── 📄 pwm.h            # PWM output control
│   │   ├── 📄 fan.h            # Fan control
│   │   └── 📄 harness.h        # Vehicle harness detection
│   ├── 📁 stm32f4/            # STM32F413 specific code
│   │   ├── 📄 board.h          # F4 board configuration
│   │   ├── 📄 clock.h          # Clock configuration
│   │   ├── 📄 startup_stm32f413xx.s # Assembly startup code
│   │   ├── 📄 stm32f4_flash.ld # Linker script
│   │   └── 📁 inc/             # STM32F4 CMSIS headers
│   ├── 📁 stm32h7/            # STM32H725 specific code
│   │   ├── 📄 board.h          # H7 board configuration
│   │   ├── 📄 clock.h          # Clock configuration
│   │   ├── 📄 startup_stm32h7x5xx.s # Assembly startup code
│   │   ├── 📄 stm32h7x5_flash.ld # Linker script
│   │   └── 📁 inc/             # STM32H7 CMSIS headers
│   ├── 📁 jungle/             # Panda Jungle firmware (separate device)
│   │   ├── 📄 main.c           # Jungle-specific main firmware
│   │   ├── 📁 boards/          # Jungle board variants
│   │   └── 📁 scripts/         # Jungle testing scripts
│   └── 📁 recover/            # Firmware recovery tools
│       ├── 📄 recover.py       # Main recovery script
│       └── 📄 dfu.py           # DFU mode utilities
├── 📁 python/                  # Host Communication Library
│   ├── 📄 __init__.py          # Main Panda class interface and API
│   ├── 📄 base.py              # Base handle class for device communication
│   ├── 📄 usb.py               # USB communication protocol implementation
│   ├── 📄 spi.py               # SPI communication protocol implementation
│   ├── 📄 dfu.py               # Device Firmware Update functionality
│   ├── 📄 constants.py         # System constants and definitions
│   └── 📄 utils.py             # Utility functions
├── 📁 tests/                   # Comprehensive Test Suite
│   ├── 📁 hitl/                # Hardware-in-the-Loop Tests
│   │   ├── 📄 1_program.py     # Firmware programming test
│   │   ├── 📄 2_usb_protocol.py # USB protocol validation
│   │   ├── 📄 3_can_loopback.py # CAN loopback testing
│   │   ├── 📄 4_can_health.py   # CAN health monitoring
│   │   ├── 📄 5_spi_protocol.py # SPI protocol testing
│   │   ├── 📄 6_dfu_update.py   # DFU update testing
│   │   ├── 📄 7_power_modes.py  # Power mode testing
│   │   ├── 📄 8_safety_model.py # Safety model validation
│   │   ├── 📄 9_harness.py      # Harness detection testing
│   │   ├── 📄 run_parallel_tests.sh # Parallel test execution
│   │   ├── 📄 run_serial_tests.sh   # Serial test execution
│   │   └── 📁 known_bootstub/   # Reference bootloader binaries
│   ├── 📁 libpanda/            # C Library Tests
│   │   ├── 📄 panda.c          # C library implementation
│   │   └── 📄 libpanda_py.py   # Python bindings for C library
│   ├── 📁 misra/               # MISRA C:2012 Compliance Testing
│   │   ├── 📄 test_misra.sh    # MISRA compliance checker
│   │   ├── 📄 coverage_table   # MISRA rule coverage tracking
│   │   └── 📄 suppressions.txt # Approved rule suppressions
│   └── 📁 usbprotocol/         # USB Protocol Tests
│       ├── 📄 test_comms.py    # USB communication tests
│       └── 📄 test_pandalib.py # Panda library tests
├── 📁 scripts/                 # Development and Testing Scripts
│   ├── 📄 loopback_test.py     # CAN loopback testing
│   ├── 📄 health_test.py       # Device health monitoring
│   ├── 📄 can_health.py        # CAN bus health analysis
│   ├── 📄 debug_console.py     # Debug console interface
│   ├── 📄 get_version.py       # Firmware version query
│   ├── 📄 relay_test.py        # Relay functionality testing
│   ├── 📁 fan/                 # Fan control scripts
│   └── 📁 development/         # Development utilities
├── 📁 examples/                # Usage Examples
│   ├── 📄 can_logger.py        # CAN message logging example
│   ├── 📄 tesla_tester.py      # Tesla-specific testing
│   ├── 📄 query_fw_versions.py # Firmware version querying
│   └── 📄 can_bit_transition.py # CAN timing analysis
├── 📁 crypto/                  # Cryptographic Components
│   ├── 📄 rsa.c/.h             # RSA cryptographic implementation
│   ├── 📄 sha.c/.h             # SHA hashing implementation
│   └── 📄 sign.py              # Firmware signing utility
├── 📁 certs/                   # Certificates and Keys
│   ├── 📄 debug                # Debug build certificate
│   ├── 📄 debug.pub            # Debug public key
│   └── 📄 release.pub          # Release public key
├── 📁 drivers/                 # System Drivers
│   └── 📁 spi/                 # Linux SPI kernel driver
│       ├── 📄 spidev_panda.c   # Modified SPI device driver
│       └── 📄 Makefile         # Driver build configuration
├── 📁 docs/                    # Documentation Assets
│   ├── 📄 CANPacket_structure.png # CAN packet format diagram
│   └── 📄 USB_packet_structure.png # USB packet format diagram
├── 📄 SConstruct               # Main build system entry point
├── 📄 SConscript               # Detailed build configuration
├── 📄 pyproject.toml           # Python package configuration
├── 📄 README.md                # Project overview and usage instructions
├── 📄 CLAUDE.md                # AI assistant guidance
├── 📄 communication.md         # System communication architecture
├── 📄 protocol.md              # USB protocol specification
├── 📄 feature.md               # Feature documentation
├── 📄 setup.sh                 # Environment setup script
├── 📄 test.sh                  # Complete test suite runner
├── 📄 Dockerfile               # Container build configuration
└── 📄 Jenkinsfile              # CI/CD pipeline configuration
```

## Core Architecture Components

### 1. **Firmware Layer (`/board/`)**
**Purpose**: Embedded C firmware running on STM32 microcontrollers

**Key Responsibilities**:
- **CAN/CAN-FD Communication**: Managing up to 3 CAN buses with different transceivers
- **USB Communication**: Full-speed USB 2.0 interface for host communication
- **Safety Validation**: Implementing vehicle-specific safety models
- **Hardware Abstraction**: Providing unified interface across different STM32 variants
- **Power Management**: Handling 12V automotive power and USB power sources
- **Harness Detection**: Automatic detection of vehicle harness connection and orientation

**Hardware Variants**:
- **Black Panda**: Original STM32F413 based design
- **Red Panda**: STM32H725 based design with CAN-FD support
- **White/Tres/Cuatro/Dos**: Various hardware revisions and configurations
- **Jungle**: Multi-CAN debug interface for advanced testing

### 2. **Host Library (`/python/`)**
**Purpose**: Python library for host computer communication with panda devices

**Key Features**:
- **Multi-Protocol Support**: USB, SPI, and DFU communication modes
- **CAN Message Handling**: Packing/unpacking CAN frames with proper checksums
- **Device Management**: Automatic device discovery and connection handling
- **Safety Integration**: Safety model configuration and validation
- **Firmware Updates**: DFU-based firmware update functionality

### 3. **Quality Assurance (`/tests/`)**
**Purpose**: Comprehensive testing framework ensuring firmware and software quality

**Testing Layers**:
- **Hardware-in-the-Loop (HITL)**: Real hardware testing with physical CAN buses
- **Unit Testing**: Individual component validation
- **Protocol Testing**: USB and SPI communication validation
- **Safety Testing**: Safety model verification
- **Compliance Testing**: MISRA C:2012 compliance validation

### 4. **Development Tools (`/scripts/`, `/examples/`)**
**Purpose**: Development utilities and usage examples

**Categories**:
- **Diagnostic Scripts**: Health monitoring and system analysis
- **Test Scripts**: Specialized testing for different scenarios
- **Development Tools**: Debug console, version querying
- **Usage Examples**: Real-world usage patterns and implementations

### 5. **Security Layer (`/crypto/`, `/certs/`)**
**Purpose**: Cryptographic security for firmware integrity

**Components**:
- **RSA/SHA Implementation**: Hardware-accelerated cryptography
- **Certificate Management**: Debug and release certificate handling
- **Firmware Signing**: Secure firmware validation and loading

## Build System Architecture

### **SCons Build System**
- **Cross-compilation**: ARM toolchain for STM32 targets
- **Dual-target support**: STM32F4 (F413) and STM32H7 (H725)
- **Build modes**: Debug (with debug certificates) and Release (requires CERT env var)
- **Strict compilation**: `-Wall -Wextra -Wstrict-prototypes -Werror`
- **MISRA compliance**: cppcheck with MISRA C:2012 addon

### **Python Package Management**
- **uv**: Modern Python dependency management
- **Development dependencies**: Testing, linting, and type checking tools
- **Build isolation**: Separate development and runtime environments

## Communication Architecture

### **Why STM32 Uses Python (Architecture Explanation)**

The STM32 microcontroller doesn't actually "use" Python - there's an important architectural separation:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         ARCHITECTURE SEPARATION                             │
└─────────────────────────────────────────────────────────────────────────────┘

Host Computer (Python)                     STM32 Microcontroller (C)
┌─────────────────────────┐                ┌─────────────────────────┐
│                         │                │                         │
│  🐍 Python Application  │                │  ⚙️  C Firmware         │
│  ┌─────────────────────┤                │  ┌─────────────────────┤
│  │ • Easy Development  │                │  │ • Real-time Control │
│  │ • Rich Libraries    │                │  │ • Memory Constrained│
│  │ • Cross-platform    │                │  │ • Hardware Access   │
│  │ • Rapid Prototyping │                │  │ • Safety Critical   │
│  └─────────────────────┤                │  └─────────────────────┤
│                         │                │                         │
│  Resources:             │                │  Resources:             │
│  • GB of RAM           │                │  • KB of RAM           │
│  • GHz CPU             │                │  • MHz CPU             │
│  • Full OS             │                │  • Bare Metal          │
│  • No Real-time Req.   │                │  • Real-time Critical  │
│                         │                │                         │
└─────────────────────────┘                └─────────────────────────┘
              │                                          │
              │              USB Protocol                │
              │ ◄──────────────────────────────────────► │
              │                                          │
┌─────────────▼─────────────┐                ┌─────────▼─────────────┐
│     Python Library        │                │    STM32 Firmware     │
│  ┌─────────────────────┐  │                │  ┌─────────────────┐  │
│  │ panda.can_send()    │  │                │  │ USB Handler     │  │
│  │ panda.can_recv()    │  │                │  │ CAN Controller  │  │
│  │ panda.set_safety()  │  │                │  │ Safety Hooks    │  │
│  └─────────────────────┘  │                │  └─────────────────┘  │
└───────────────────────────┘                └───────────────────────┘
```

### **Protocol Stack**
```
┌─────────────────────────────────────────────────────────────┐
│ Application Layer (Python/C++)                             │
├─────────────────────────────────────────────────────────────┤
│ Panda Library (python/ directory)                          │
├─────────────────────────────────────────────────────────────┤
│ USB Protocol Layer (protocol.md specification)             │
├─────────────────────────────────────────────────────────────┤
│ STM32 Firmware (board/ directory)                          │
├─────────────────────────────────────────────────────────────┤
│ Hardware Abstraction Layer (drivers/ subdirectory)         │
├─────────────────────────────────────────────────────────────┤
│ CAN/CAN-FD Physical Layer (Vehicle Network)                │
└─────────────────────────────────────────────────────────────┘
```

### **Communication Flow**
```
Python App (Host) ←→ USB Protocol ←→ STM32 Firmware ←→ CAN Bus ←→ Vehicle

Example:
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ panda.can_send()│───►│ USB Control Cmd │───►│ Safety Check    │───►│ Vehicle CAN Bus │
│ (Python)        │    │ (USB Protocol)  │    │ (STM32 C Code)  │    │ (Physical)      │
└─────────────────┘    └─────────────────┘    └─────────────────┘    └─────────────────┘
```

### **Why Python is Used (On Host Side)**

1. **Automotive Integration**: Works with opendbc (CAN database) and openpilot (self-driving software)
2. **Ease of Development**: Simple API for complex automotive protocols
3. **Cross-Platform**: Windows, Linux, macOS support
4. **Rapid Prototyping**: Quick testing and debugging
5. **comma.ai Ecosystem**: Primary language for their automotive software stack

### **Data Flow Examples**
```python
# Python code (runs on computer)
panda = Panda()
panda.can_send(0x123, b'data', 0)  # Send CAN message
msgs = panda.can_recv()            # Receive CAN messages
```

**What happens internally:**
1. **Python Library** → Formats CAN message with USB protocol
2. **USB Transfer** → Sends command to STM32
3. **STM32 Firmware** → Validates message with safety hooks
4. **CAN Controller** → Transmits on vehicle CAN bus
5. **Vehicle** → Receives and processes CAN message

## Safety and Compliance

### **Safety-Critical Standards**
- **MISRA C:2012**: Static analysis for automotive safety
- **Functional Safety**: Vehicle-specific safety models
- **Code Quality**: Strict compiler warnings and error handling
- **Testing Rigor**: Multi-layer testing with physical hardware validation

### **Security Features**
- **Firmware Signing**: RSA-based firmware validation
- **Certificate Management**: Separate debug and release certificates
- **Secure Boot**: Bootloader validation of firmware integrity
- **Communication Security**: Checksum validation of all messages

## Project Development Workflow

### **Build Process**
1. **Environment Setup**: `./setup.sh` - Install dependencies
2. **Firmware Build**: `scons -j8` - Compile STM32 firmware
3. **Testing**: `./test.sh` - Run complete test suite
4. **Quality Checks**: `ruff check .` and `mypy python/` - Code quality validation

### **Development Cycle**
1. **Feature Development**: Implement in both firmware and Python library
2. **Unit Testing**: Validate individual components
3. **Integration Testing**: Test with physical hardware
4. **Safety Validation**: Verify safety model compliance
5. **Quality Assurance**: MISRA compliance and code review

This directory structure supports the development of safety-critical automotive firmware while maintaining high code quality standards and comprehensive testing coverage. The modular design allows for clear separation of concerns between embedded firmware, host communication, testing, and development tools.