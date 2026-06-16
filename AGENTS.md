# Repository Guidelines

## Project Structure & Module Organization
This repository contains PlatformIO firmware for an ESP32-S3 mecanum car.

- `src/` - application source files (`main.cpp`, `motor_control.cpp`)
- `include/` - shared headers and hardware/network configuration (`config.h`, `motor_control.h`)
- `lib/` - reserved for reusable libraries; keep custom modules here if they outgrow `src/`
- `test/` - PlatformIO unit tests
- `platformio.ini` - board, framework, upload, and monitor configuration
- `.pio/` - generated build output; do not edit manually

Keep motor logic in `src/motor_control.cpp` and board-specific constants in `include/config.h`.

## Build, Test, and Development Commands
Run commands from the repository root:

- `pio run` - build the firmware
- `pio run --target upload` - flash firmware to the ESP32-S3
- `pio device monitor --baud 115200` - open the serial monitor
- `pio test` - run PlatformIO tests in `test/`
- `pio run -t clean` - remove generated build artifacts

Example workflow: build, upload, then monitor logs to verify AP startup and UDP command handling.

## Coding Style & Naming Conventions
Use C++ with Arduino/PlatformIO conventions already present in the codebase:

- Indentation: 2 spaces, no tabs
- Braces: opening brace on the same line
- File names: lowercase with underscores (for example `motor_control.cpp`)
- Functions: `camelCase` (`startUdpServer`, `handleCommand`)
- Constants/macros: `UPPER_SNAKE_CASE` (`UDP_PORT`, `COMMAND_TIMEOUT_MS`)

Prefer small, single-purpose functions. Keep command decoding centralized in `handleCommand()`.

## Testing Guidelines
Use the PlatformIO Test Runner for unit and integration-style embedded tests.

- Place tests under `test/`
- Group related checks by feature (for example `test_motor_commands/`)
- Name tests to reflect behavior, such as `test_failsafe_stops_on_timeout`

Before opening a PR, at minimum run `pio run` and `pio test` if tests exist.

## Commit & Pull Request Guidelines
Git history is not available in this workspace, so no repository-specific commit pattern could be verified. Use clear, imperative commit messages such as:

- `Add UDP command validation`
- `Fix mecanum strafe mapping`

For pull requests, include:
- a short summary of the change
- affected modules/files
- hardware impact (pins, PWM, Wi-Fi, UDP behavior)
- test evidence (`pio run`, `pio test`, serial logs)

## Security & Configuration Tips
Do not commit real production credentials. Review `include/config.h` before deployment, especially Wi-Fi SSID/password, UDP port, pin mappings, and motor direction wiring.
