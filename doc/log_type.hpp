#ifndef LOG_TYPE_HPP
#define LOG_TYPE_HPP

// 前方宣言
namespace logger {
class Logger;
class LoggerConfig;
}  // namespace logger

/**
 * @brief ログレベル列挙型
 * @details ログメッセージの重要度を定義
 */
enum class LogLevel {
    DEBUG_,  ///< デバッグレベル - 詳細な開発情報
    INFO_,   ///< 情報レベル - 一般的な情報
    WARN_,   ///< 警告レベル - 注意が必要な状況
    ERROR_   ///< エラーレベル - エラー情報
};

namespace logger {
/**
 * @brief ログエントリ構造体
 * @details 単一のログメッセージに関する全情報を格納
 * @todo 拡張
 */
struct LogEntry {
    LogLevel level;        ///< ログレベル
    const char* filename;  ///< ソースファイル名
    int line;              ///< 行番号
    const char* message;   ///< ログメッセージ
    char* formatedMsg;
    // const char* function;  ///< 関数名（将来用）
    // timestamp_t timestamp; ///< タイムスタンプ（将来実装）
};

/**
 * @brief カラーコードマップ
 * @details カラータグとANSIコードの対応表（一元管理）
 */
namespace ColorMap {
/**
 * @brief ANSIカラーコードマップ
 */
static const std::map<char, const char*> ANSI_COLORS = {
    // 基本色
    {'r', "\033[31m"},  // Red - 赤
    {'g', "\033[32m"},  // Green - 緑
    {'y', "\033[33m"},  // Yellow - 黄色
    {'b', "\033[34m"},  // Blue - 青
    {'p', "\033[35m"},  // Purple - 紫

    // 明るい色
    {'a', "\033[36m"},  // Aqua (Cyan) - 明るい水色
    {'l', "\033[92m"},  // Lime (Bright Green) - 明るい緑
    {'s', "\033[97m"},  // Silver (Bright White) - 明るい灰色

    // 暗い色 / その他
    {'m', "\033[91m"},  // Maroon (Bright Red) - 暗い赤
    {'o', "\033[93m"},  // Olive (Bright Yellow) - くすんだ黄緑
    {'t', "\033[96m"},  // Teal (Bright Cyan) - 青緑
    {'n', "\033[94m"},  // Navy (Bright Blue) - 暗い青
    {'f', "\033[95m"},  // Fuchsia (Bright Magenta) - 派手なピンク

    // 特殊色
    {'z', "\033[90m"},  // Gray - ねずみ色
    {'w', "\033[37m"},  // White - 白
    {'k', "\033[30m"},  // Black - 黒
};

/**
 * @brief ログレベル用カラーマップ
 */
static const std::map<LogLevel, const char*> LEVEL_COLORS = {
    {LogLevel::DEBUG_, ANSI_COLORS.at('b')},  // Blue
    {LogLevel::INFO_, ANSI_COLORS.at('g')},   // Green
    {LogLevel::WARN_, ANSI_COLORS.at('y')},   // Yellow
    {LogLevel::ERROR_, ANSI_COLORS.at('r')}   // Red
};

/**
 * @brief リセットコード
 */
static const char* RESET = "\033[0m";
}  // namespace ColorMap

}  // namespace logger

#endif  // LOG_TYPE_HPP