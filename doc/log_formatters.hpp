/**
 * @file formatters.hpp
 * @brief ログフォーマット専用ライブラリ
 * @details 様々な出力形式に対応するフォーマッタクラス群
 * @author ren255
 */

#ifndef LOG_FORMATTERS_HPP
#define LOG_FORMATTERS_HPP

#include "logger.hpp"
#include <cstring>
#include <cstdio>

namespace logger {
/**
 * @brief フォーマット機能を提供する名前空間
 */
namespace Formatters {

/**
 * @brief フォーマッタ基底抽象クラス
 * @details 共通ロジックも提供
 */
class FormatterBase {
   public:
    struct LogInfo {
        const char* level_str;
        const char* filename;
    };

    virtual ~FormatterBase() = default;

    /**
     * @brief ログエントリをフォーマット
     * @param entry ログエントリ（formatedMsgバッファに出力される）
     */
    virtual void format(const LogEntry& entry) = 0;

   protected:
    /**
     * @brief レベル文字列をパディング
     * @param level_str レベル文字列
     * @param output 出力バッファ（target_width+1以上のサイズが必要）
     * @param target_width 目標幅（デフォルト8）
     */
    void format_level_padding(const char* level_str, char* output,
                              int target_width = 8) {
        char bracketed[32];
        snprintf(bracketed, sizeof(bracketed), "[%s]", level_str);
        Utils::StringUtils::apply_padding(bracketed, target_width, output);
    }

    /**
     * @brief ファイル名:行番号をパディング
     * @param filename ファイル名
     * @param line 行番号
     * @param output 出力バッファ（target_width+1以上のサイズが必要）
     * @param target_width 目標幅（デフォルト15）
     */
    void format_location_padding(const char* filename, int line, char* output,
                                 int target_width = 15) {
        char location[64];
        snprintf(location, sizeof(location), "%s:%d", filename, line);
        Utils::StringUtils::apply_padding(location, target_width, output, true);
    }

    LogInfo get_LogInfo(const LogEntry& entry) {
        const char* level_str =
            Utils::StringUtils::get_level_string(entry.level);
        const char* filename =
            Utils::StringUtils::extract_filename(entry.filename);
        return {level_str, filename};
    }
};

/**
 * @brief コンソール用フォーマッタ
 * @details カラー付きでコンソールに適した形式でフォーマット
 */
class ConsoleFmt : public FormatterBase {
   private:
    bool color_enabled;

   public:
    /**
     * @brief コンストラクタ
     * @param enable_color カラー出力を有効にするか
     */
    explicit ConsoleFmt(bool enable_color = true)
        : color_enabled(enable_color) {}

    /**
     * @brief ログエントリをコンソール形式でフォーマット
     * @param entry ログエントリ（formatedMsgバッファに出力される）
     */
    void format(const LogEntry& entry) override {
        // Utils::ColorHelperとUtils::StringUtilsを使用して統一処理
        LogInfo info = this->get_LogInfo(entry);

        const char* color = ColorMap::LEVEL_COLORS.at(entry.level);
        const char* reset = ColorMap::RESET;

        // カラーメッセージを解析（統一処理を使用）
        char colored_msg[256];
        Utils::ColorHelper::parse_color_tags(entry.message, colored_msg,
                                             sizeof(colored_msg));

        // レベル部分をパディング（8文字固定）
        char level_padded[16];
        const int level_padding = 8 - strlen(info.level_str) - 2;
        snprintf(level_padded, sizeof(level_padded), "[%s]%-*s", info.level_str,
                 level_padding, "");

        // 最終フォーマット: [LEVEL]   filename:line        : message
        const int line_width = snprintf(nullptr, 0, "%d", entry.line);
        const int padding = 13 - strlen(info.filename) - line_width;
        Utils::StringUtils::log_sprintf(entry, "%s%s%s %s:%d%*s : %s", color,
                                        level_padded, reset, info.filename,
                                        entry.line, padding, "", colored_msg);
    }
};

/**
 * @brief プレーンテキストフォーマッタ
 * @details シンプルなテキスト形式（カラーなし）
 */
class PlainFmt : public FormatterBase {
   public:
    /**
     * @brief ログエントリをプレーンテキスト形式でフォーマット
     * @param entry ログエントリ（formatedMsgバッファに出力される）
     */
    void format(const LogEntry& entry) override {
        LogInfo info = this->get_LogInfo(entry);

        // プレーンテキストではカラータグを除去
        char plain_message[256];
        Utils::ColorHelper::strip_color_tags(entry.message, plain_message,
                                             sizeof(plain_message));

        // シンプルなフォーマット
        Utils::StringUtils::log_sprintf(entry, "[%s] %s:%d : %s",
                                        info.level_str, info.filename,
                                        entry.line, plain_message);
    }
};

}  // namespace Formatters
}  // namespace logger
#endif  // LOG_FORMATTERS_HPP