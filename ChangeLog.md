# ChangeLog<!--! {#change_log} -->

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its better-defined brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

- Refactored register and stream I/O to use Adafruit BusIO abstractions across the library.
- Reworked class architecture around a dedicated `SC16IS7xx_UART` channel class with chip wrappers/traits for multiple SC16IS7xx variants.
- Updated public chip typing from a single concrete focus to trait-driven concrete aliases (`SC16IS740`, `SC16IS750`, `SC16IS760`, `SC16IS752`, `SC16IS762`).
- Updated UART channel ownership to fixed in-class storage and constrained channel construction/rebinding to the owning `SC16IS7xx` class.
- Updated UART channel accessors so `getUART(channel)` remains pointer-based for indexed lookup while `uartA()` / `uartB()` provide direct references.
- Renamed/standardized several APIs for cross-core compatibility, including `pinModeExternal` / `digitalReadExternal` / `digitalWriteExternal` and pin interrupt attach/detach naming.
- Revised interrupt handling flow and callback dispatch to support multi-object usage and avoid destructive re-read behavior where possible.
- Updated default-address/default-value handling with clearer macro guards and defaults.
- Updated example sketches, README, and documentation structure to match the new API layout and naming.
- Applied broad formatting, spelling, and doc clarity cleanups across source and docs.
- Updated project metadata and automation files, including release/docs workflow structure.

### Added

- Support for attaching and handling all supported interrupt sources, including modem/data/line and pin-related callbacks.
- Hardware flow-control support and related modem interrupt documentation/handling improvements.
- Typed serial line-configuration support via `SC16IS7xxSerialConfig`.
- Compile-time chip trait model (`SC16IS7xxChipTraits`) and trait-bound chip wrapper template (`SC16IS7xxChip`).
- Doxygen project scaffolding and expanded API/docs coverage.
- Code Rabbit configuration and additional repository/workflow scaffolding aligned with EnviroDIY standards.

### Removed

- Removed the legacy `extern` serial interface pattern used by examples.
- Removed non-functional `getLastInterruptPin` logic.

### Fixed

- Restored/fixed `peek()` and read behavior based on the [TD-er](https://github.com/TD-er/SC16IS752) approach and subsequent FIFO-read correctness fixes.
- Fixed channel-specific register addressing/bit handling in UART and interrupt paths.
- Fixed FIFO control/trigger-level behavior, flow-control enable paths, and divisor/baud-related edge cases (including overflow and zero-baud guard).
- Fixed signed/unsigned and pin-number overflow issues found during cleanup and review.
- Fixed interrupt pending interpretation, mask handling, latched-state processing, and pin-change handling edge cases.
- Fixed cross-core naming conflicts (ESP32/Pico-related symbols and API names).
- Fixed Doxygen completeness issues (including constructor/member doc warnings) and multiple missing doc blocks.

***

## 1.0.0

Initial fork from Appnostic

***

[Unreleased]: https://github.com/EnviroDIY/SC16IS7XX/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/EnviroDIY/SC16IS7XX/releases/tag/v1.0.0

<!--! @tableofcontents{HTML:1} -->

<!--! @m_footernavigation -->
