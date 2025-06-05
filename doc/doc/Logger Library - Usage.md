# Logger Library Technical Specification

## Overview
C++11準拠の高機能ログライブラリ。カラータグ、複数フォーマット、コンパイル時検証を提供。

## Core Architecture

### Log Levels
```cpp
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };
```

### Main API
```cpp
// シングルトンアクセス
logger::Logger& get_logger();
logger::LoggerConfig& get_logger_config();

// マクロAPI（推奨）
LOG_DEBUG("Debug: %s", value);
LOG_INFO("Info message");
LOG_WARNING("Warning: %d", code);
LOG_ERROR("Error occurred");
```

### Basic Usage
```cpp
// 設定
get_logger().set_level(LogLevel::DEBUG);

// カラータグ使用
LOG_INFO("Success: g|緑色メッセージ| and r|赤色警告|");

// フォーマッタ変更
get_logger().set_formatter(std::make_unique<JsonFormatter>());
```

## Color Tag System

### Syntax
- `x|message|` - カラー開始/終了
- `||` - リテラル`|`をエスケープ

### Color Codes
| Tag | Color | ANSI Code |
|-----|-------|-----------|
| `r` | Red   | `\033[31m` |
| `g` | Green | `\033[32m` |
| `y` | Yellow| `\033[33m` |
| `b` | Blue  | `\033[34m` |
| `d` | Reset | `\033[0m`  |

### Validation
- **Compile-time**: `static_assert`でタグペアリング検証
- **Runtime**: 実行時に不正タグを検出してエラー出力

## Formatters

### Available Types
```cpp
// Console (default)
ConsoleFormatter(bool colored = true)

// Structured formats
JsonFormatter()     // {"level":"INFO","file":"main.cpp",...}
PlainFormatter()    // [INFO] main.cpp:42 : message
CsvFormatter()      // "INFO","main.cpp",42,"message"
XmlFormatter()      // <log level="INFO" file="main.cpp" line="42">message</log>
```

### Custom Formatter
```cpp
class CustomFormatter : public IFormatter {
    void format(const LogEntry& entry, char* output, int max_len) override {
        // Implementation
    }
};
```

## Writers

### Built-in Writers
```cpp
ConsoleWriter()           // stdout出力
BufferedWriter(writer)    // バッファリング付き出力
```

### Custom Writer
```cpp
class FileWriter : public IWriter {
    void write(const char* message) override {
        // File output implementation
    }
};
```

## Configuration

### Logger Setup
```cpp
auto& logger = get_logger();
logger.set_level(LogLevel::DEBUG);
logger.set_formatter(std::make_unique<JsonFormatter>());
logger.set_writer(std::make_unique<FileWriter>("app.log"));
```

### Global Config
```cpp
auto& config = get_logger_config();
config.set_min_level(LogLevel::WARNING);
config.set_color_enabled(false);
```

## Technical Details

### Memory Management
- RAII準拠、スマートポインタ使用
- 固定サイズバッファ（256/512bytes）でスタック安全

### Thread Safety
- **非対応** - 呼び出し側で排他制御が必要

### Performance
- コンパイル時検証によりランタイムオーバーヘッド最小化
- バッファリング機能で I/O 効率化

### Error Handling
- 不正カラータグ → ERRORレベルで警告出力
- バッファオーバーフロー → 切り捨て処理
- フォーマッタ/ライター未設定 → デフォルト動作

## File Structure
```
logger.hpp          # Public API + マクロ定義
log_type.hpp        # 型定義・列挙型
log_core.hpp        # Loggerクラス実装
log_utils.hpp       # ユーティリティクラス群
log_formatters.hpp  # フォーマッタ実装
log_writers.hpp     # ライター実装
```

## Limitations
- C++11以上必須
- スレッドセーフ未対応
- タイムスタンプ機能未実装（将来対応予定）
- 固定バッファサイズ制限

## Integration
ヘッダーオンリーライブラリ。`#include "logger.hpp"`のみで使用可能。

```pu
classDiagram
    %% Enums
    class LogLevel {
        <<enumeration>>
        DEBUG
        INFO
        WARNING
        ERROR
    }

    %% Core Data Structures
    class LogEntry {
        +LogLevel level
        +const char* filename
        +int line
        +const char* function
        +const char* message
    }

    %% Configuration
    class LoggerConfig {
        +LogLevel min_level
        +bool color_enabled
        +bool show_timestamp
        +set_min_level(LogLevel level)
        +set_color_enabled(bool enabled)
    }

    %% Main Logger Class
    class Logger {
        -LogLevel current_level
        -unique_ptr~IFormatter~ formatter
        -unique_ptr~IWriter~ writer
        -log_internal(LogLevel level, const char* file, int line, const char* message)
        +Logger()
        +set_level(LogLevel level)
        +get_level() LogLevel
        +set_formatter(unique_ptr~IFormatter~ formatter)
        +set_writer(unique_ptr~IWriter~ writer)
        +debug(const char* file, int line, const char* fmt, ...)
        +info(const char* file, int line, const char* fmt, ...)
        +warning(const char* file, int line, const char* fmt, ...)
        +error(const char* file, int line, const char* fmt, ...)
    }

    %% Formatter Interface and Implementations
    class IFormatter {
        <<interface>>
        +format(const LogEntry& entry, char* output, int max_len)*
    }

    class ConsoleFormatter {
        -bool color_enabled
        +ConsoleFormatter(bool colored)
        +format(const LogEntry& entry, char* output, int max_len)
    }

    class JsonFormatter {
        +format(const LogEntry& entry, char* output, int max_len)
    }

    class PlainFormatter {
        +format(const LogEntry& entry, char* output, int max_len)
    }

    class CsvFormatter {
        +format(const LogEntry& entry, char* output, int max_len)
    }

    class XmlFormatter {
        +format(const LogEntry& entry, char* output, int max_len)
    }

    %% Writer Interface and Implementations
    class IWriter {
        <<interface>>
        +write(const char* message)*
    }

    class ConsoleWriter {
        +write(const char* message)
    }

    class BufferedWriter {
        -static const int BUFFER_SIZE
        -char buffer[BUFFER_SIZE]
        -int buffer_pos
        -unique_ptr~IWriter~ underlying_writer
        +BufferedWriter(unique_ptr~IWriter~ writer)
        +write(const char* message)
        +flush()
        +~BufferedWriter()
    }

    %% Utility Classes
    class ColorHelper {
        <<utility>>
        +get_level_color(LogLevel level, bool color_enabled)$ const char*
        +get_reset_color(bool color_enabled)$ const char*
        +parse_color_tags(const char* input, char* output, int max_len, bool color_enabled)$
        +strip_color_tags(const char* input, char* output, int max_len)$
    }

    class ValidationUtils {
        <<utility>>
        +check_colors_ct(const char* input)$ constexpr bool
        +validate_color_tags_runtime(const char* input)$ bool
    }

    class StringUtils {
        <<utility>>
        +extract_filename(const char* filepath)$ const char*
        +get_level_string(LogLevel level)$ const char*
        +safe_strcpy(char* dest, const char* src, int max_len)$
    }

    %% Color Map (Static Data)
    class ColorMap {
        <<utility>>
        +ANSI_COLORS$ map~char, const char*~
        +LEVEL_COLORS$ map~LogLevel, const char*~
        +RESET$ const char*
    }

    %% Global Functions
    class GlobalFunctions {
        <<utility>>
        +get_logger()$ Logger&
        +get_logger_config()$ LoggerConfig&
    }

    %% Relationships
    Logger ||--o{ LogLevel : uses
    Logger ||--|| IFormatter : composition
    Logger ||--|| IWriter : composition
    Logger ..> LogEntry : creates

    IFormatter <|.. ConsoleFormatter : implements
    IFormatter <|.. JsonFormatter : implements
    IFormatter <|.. PlainFormatter : implements
    IFormatter <|.. CsvFormatter : implements
    IFormatter <|.. XmlFormatter : implements

    IWriter <|.. ConsoleWriter : implements
    IWriter <|.. BufferedWriter : implements
    BufferedWriter ||--|| IWriter : composition

    Logger ..> ColorHelper : uses
    Logger ..> ValidationUtils : uses
    Logger ..> StringUtils : uses

    ConsoleFormatter ..> ColorHelper : uses
    ConsoleFormatter ..> StringUtils : uses
    JsonFormatter ..> StringUtils : uses
    PlainFormatter ..> StringUtils : uses
    CsvFormatter ..> StringUtils : uses
    XmlFormatter ..> StringUtils : uses

    ColorHelper ..> ColorMap : uses
    ValidationUtils ..> ColorMap : uses

    GlobalFunctions ..> Logger : creates
    GlobalFunctions ..> LoggerConfig : creates

    %% Package Organization
    namespace logger {
        class Logger
        class LoggerConfig
        class LogEntry
        class ColorMap
    }

    namespace "logger::Formatters" {
        class IFormatter
        class ConsoleFormatter
        class JsonFormatter
        class PlainFormatter
        class CsvFormatter
        class XmlFormatter
    }

    namespace "logger::Writers" {
        class IWriter
        class ConsoleWriter
        class BufferedWriter
    }

    namespace "logger::Utils" {
        class ColorHelper
        class ValidationUtils
        class StringUtils
    }
```