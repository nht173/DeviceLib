# Changelog

All notable changes to DeviceLib will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.1] - 2025-07-23

### Added
- Virtual destructors for `GenericInput`, `GenericButton`, and `GenericOutput` classes
- Timer cleanup on object destruction to prevent memory leaks
- Null pointer validation in timer callbacks
- Enhanced error reporting with `esp_err_to_name()` for ESP32
- Timer null checks before operations
- `clearCallbacks()` method for `GenericButton` to remove all callbacks
- `removeCallbacks(event_type)` method for selective callback removal
- Cross-platform destructor support (ESP32 and ESP8266)

### Changed
- Updated version consistency across all header files to 1.1.1
- Improved documentation comments for deprecated methods
- Enhanced parameter validation in constructors
- Better initialization error handling

### Fixed
- Memory leaks from ESP32 `esp_timer` not being properly deleted
- Memory leaks from ESP8266 `Ticker` not being detached
- Race conditions during object destruction
- Timer callback execution safety
- Potential crashes when timer callbacks execute during object destruction

### Technical Details
- **Memory Management**: Implemented proper RAII pattern with virtual destructors
- **Cross-platform**: Enhanced support for both ESP32 (`esp_timer`) and ESP8266 (`Ticker`)
- **Thread Safety**: Added defensive programming with null pointer checks
- **API Consistency**: Uniform error handling and parameter validation

### Backward Compatibility
- ✅ All existing code continues to work without changes
- ✅ No breaking changes to public APIs
- ✅ Deprecated methods still functional with compiler warnings

### Testing
- Added comprehensive destructor test suite
- Memory leak prevention testing
- Cross-platform compatibility verification

## [1.1.0] - Previous Release
### Initial Features
- Event-driven button handling
- Generic output management
- PCF8574/8575 I2C expander support
- Timer-based callbacks
- Multi-platform support (ESP32/ESP8266)

## [1.0.0] - Initial Release
### Added
- Basic GenericInput/GenericButton/GenericOutput classes
- Event callback system
- Debouncing functionality
- Basic timer management
