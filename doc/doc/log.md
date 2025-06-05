# STM32/Arduino R4 シンプルLogger ライブラリ設計書

## システム概要
STM32およびArduino R4環境向けのヘッダーオンリーログライブラリです。コンソール出力専用で、ログレベル管理と色付き出力をサポートします。

## ファイル構成
```
logger.h         // 全機能を含むヘッダーオンリーライブラリ
main.cpp         // user実装file、loggerが呼び出される
```

## アーキテクチャ設計

### 主要コンポーネント構造
```
Logger.h
├── LogLevel enum (public)
├── ColorCode enum (private)
├── ConsoleFormatter class (public)
├── Logger class (public)
└── Helper Functions (private namespace)
```

### クラス設計
各クラス内で実装も行う
inline 無し

```
logger.hpp
├── [HEADER GUARDS & INCLUDES]
│   ├── #ifndef LOGGER_HPP
│   ├── #define LOGGER_HPP
│   └── #include <stdarg.h>, <stdio.h>, etc.
│
├── [GLOBAL ENUMS] (public)
│   └── enum class LogLevel
│       ├── DEBUG
│       ├── INFO
│       ├── WARNING
│       └── ERROR
│
├── [FORWARD DECLARATIONS]
│   ├── class Logger
│   └── class ConsoleFormatter
│
├── [PRIVATE NAMESPACE]
│   └── namespace LoggerPrivate
│       ├── enum class ColorCode
│       │   ├── RED
│       │   ├── GREEN
│       │   ├── YELLOW
│       │   ├── BLUE
│       │   └── RESET
│       │
│       └── [HELPER FUNCTIONS DECLARATIONS]
│
├── [PUBLIC CLASSES]
│   ├── class ConsoleFormatter
│   │   ├── [PUBLIC MEMBERS]
│   │   └── [PRIVATE MEMBERS]
│   │
│   └── class Logger
│       ├── [PUBLIC MEMBERS]
│       └── [PRIVATE MEMBERS]
│
├── [GLOBAL FUNCTIONS]
│   └── Logger& get_logger()
│
├── [MACRO DEFINITIONS]
│   ├── #define LOG_DEBUG(fmt, ...)
│   ├── #define LOG_INFO(fmt, ...)
│   ├── #define LOG_WARNING(fmt, ...)
│   └── #define LOG_ERROR(fmt, ...)
│
└── [FOOTER]
    └── #endif // LOGGER_HPP
```




### フォーマッタクラス

#### ConsoleFormatter クラス
**公開メンバ**:
- コンストラクタで基本フォーマット設定
- `String format(LogLevel level, const String& message)`

### 列挙型とデータ構造

#### LogLevel (public)
```cpp
enum class LogLevel {
    DEBUG,
    INFO, 
    WARNING,
    ERROR
};
```

#### ColorCode (private)
```cpp
namespace LoggerPrivate {
    enum class ColorCode {
        RED,
        GREEN,
        YELLOW,
        BLUE,
        RESET
    };
}
```

## 実装方針

### ヘッダーオンリー設計
- 全ての実装を`logger.h`内にインライン関数として定義
- テンプレート使用時は完全にヘッダ内で実装
- staticローカル変数でグローバル状態管理

### メモリ効率化
- Arduino環境を考慮した最小限のメモリ使用
- Stringバッファの適切なサイズ制限
- 動的メモリ確保の最小化

```

## API仕様

### マクロAPI (public)
```cpp
#define LOG_DEBUG(fmt, ...) get_logger().debug(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) get_logger().info(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) get_logger().warning(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) get_logger().error(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
```

### セットアップAPI
```cpp
// デフォルトLogger設定
get_logger().global_lvl_set(LogLevel::INFO);

```

### 色指定記法
```cpp
LOG_INFO("g|接続成功| r|エラー数: %d| 通常テキスト", error_count);
// 出力: [緑]接続成功[赤]エラー数: 5 通常テキスト
```

## フォーマット仕様

### Logger出力フォーマット
**コンソール**: `[INFO] FileName:line : メッセージ内容`

[lvl] は左つめ6文字のこり空白
FileName:line も左つめ15文字のこり空白

## グローバルインスタンス管理

### デフォルトインスタンス取得関数
- グローバルデフォルトインスタンスへの参照を返す関数を提供

```cpp
// logger.h内
Logger& get_logger();
```

## 使用例とテストケース

### 基本使用パターン
```cpp
void setup() {
    // デフォルトインスタンス初期化
    get_logger().global_lvl_set(LogLevel::DEBUG);
    
    LOG_INFO("システム開始");
}

void loop() {
    // ログ出力（デフォルトインスタンス使用）
    LOG_DEBUG("センサー値: %d", sensor_value);
    LOG_INFO("g|接続成功| 通常メッセージ");
    LOG_WARNING("y|警告:| 温度が高めです");
    LOG_ERROR("r|エラー:| 接続失敗");
}
```

### エラーハンドリング
- 不正なフォーマット文字列の処理
- メモリ不足時の安全な動作

### リソース制限
- 最大ログメッセージ長: 256文字