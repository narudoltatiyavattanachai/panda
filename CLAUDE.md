# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the comma.ai panda project - a CAN/CAN FD interface device that runs on STM32F413 and STM32H725 microcontrollers. The project consists of embedded firmware (C) and a Python library for host communication.

## Architecture

### Core Components
- **board/**: STM32 firmware written in C
  - `main.c` - Main firmware entry point
  - `drivers/` - Hardware abstraction layer (CAN, USB, SPI, etc.)
  - `opendbc/safety/` - Safety models for different car manufacturers
  - `stm32f4/` and `stm32h7/` - MCU-specific code
- **python/**: Python library for host communication
  - `__init__.py` - Main Panda class interface
  - `usb.py`, `spi.py` - Communication protocols
  - `base.py` - Base handle class
- **tests/**: Test suite
  - `hitl/` - Hardware-in-the-loop tests
  - `libpanda/` - C library tests
  - `usbprotocol/` - Protocol tests

### Build System
- **SCons** is used for building firmware (`scons -j8`)
- **uv** for Python dependency management
- Cross-compilation with `arm-none-eabi-gcc`

## Development Commands

### Environment Setup
```bash
./setup.sh  # Install dependencies and setup virtual environment
```

### Build
```bash
scons -j8   # Build firmware (parallel with 8 jobs)
```

### Testing
```bash
./test.sh                    # Run full test suite (build + lint + test)
pytest tests/               # Run Python tests only
pytest -n0 --randomly-dont-reorganize tests/  # Run tests serially
```

### Linting
```bash
ruff check .     # Python linting
mypy python/     # Type checking
```

### Hardware Testing
```bash
# Hardware-in-the-loop tests (requires physical hardware)
cd tests/hitl/
./run_parallel_tests.sh
./run_serial_tests.sh
```

## Key Development Notes

### Firmware (C Code)
- Strict compiler flags: `-Wall -Wextra -Wstrict-prototypes -Werror`
- MISRA C:2012 compliance checked with cppcheck
- Safety-critical code - high rigor standards
- Two build types: DEBUG (with `certs/debug`) and RELEASE (requires `CERT` env var)
- Use `#ifdef STM32H7` for H7-specific code, otherwise defaults to F4

### Python Library
- Uses `libusb1` for USB communication
- Integrates with `opendbc` for safety models
- Main entry point: `from panda import Panda`
- Communication via USB, SPI, or CAN interfaces

### Testing Requirements
- All firmware changes must pass hardware-in-the-loop tests
- Python code must pass ruff and mypy checks
- Safety model changes require unit tests in `opendbc/safety/tests/`

### Special Files
- `SConscript` - Build configuration
- `pyproject.toml` - Python project configuration with dev dependencies
- Certificates in `certs/` directory for firmware signing

## Common Tasks

### Running a Single Test
```bash
pytest tests/specific_test.py::test_function
```

### Building for Different MCU
The build system automatically selects the appropriate target based on the board configuration.

### Flashing Firmware
```bash
# Recovery mode flashing
python board/recover.py

# Jungle-specific flashing
python board/jungle/recover.py
```