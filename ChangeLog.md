# ChangeLog<!--! {#change_log} -->

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its better-defined brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

- Restructured to depend on Adafruit's BusIO
- Moved all stream and Serial type functions to the SC16IS752 class
- Removed the Appnostic naming prefix from source files and symbols
- Ran clang-format on all files
- Made some spelling corrections
- Updated library metadata/versioning files and expanded CI/release workflows

### Added

- Added support for attaching and handling interrupts of all supported types
- Added function to enable hardware flow control
- Added Code Rabbit configuration
- Added Doxyfile and filled out Doxygen tags where they were missing
- Added structure and workflow matching other EnviroDIY libraries

### Removed

- Removed the extern of ExtSerial

### Fixed

- Fixed peek by re-implementing read functions as they were initially written by @TD-er

***

## 1.0.0

Initial fork from Appnostic

***

[Unreleased]: https://github.com/EnviroDIY/SC16IS7XX/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/EnviroDIY/SC16IS7XX/releases/tag/v1.0.0

<!--! @tableofcontents{HTML:1} -->

<!--! @m_footernavigation -->
