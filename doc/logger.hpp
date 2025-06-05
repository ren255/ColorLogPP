/**
 * @file logger.hpp
 * @brief C++用高機能ログライブラリ - パブリックインターフェース
 * @details ユーザー向けのメインAPIを提供
 * LOG_DEBUG("デバッグメッセージy|黄色|: %s", "詳細情報");
 * @author ren255
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdlib>  // 標準Cライブラリ（malloc, free, atoi, rand など）
#include <cstdio>   // 標準C入出力（printf, sprintf, FILE* など）
#include <cstdarg>  // 可変長引数（va_list, va_start, va_endなど）
#include <cstring>  // C文字列操作（strcpy, strcmp, strlen など）
#include <memory>   // スマートポインタ（std::unique_ptr, std::shared_ptr など）
#include <mutex>  // 排他制御（std::mutex, std::lock_guard, std::once_flag など）
#include <vector>  // 動的配列（std::vector）
#include <map>     // 連想配列（std::map, std::multimap）

constexpr size_t LOG_MSG_SIZE = 256;               // 入力メッセージ最大長
constexpr size_t LOG_FMT_SIZE = LOG_MSG_SIZE * 2;  // フォーマット後
constexpr size_t BUFFER_SIZE = 1024;               // バッファサイズ
#define COL_CHECK 1

#include "log_type.hpp"
#include "log_utils.hpp"
#include "log_writers.hpp"
#include "log_formatters.hpp"
#include "log_core.hpp"

// グローバル関数の実装
/**
 * @brief 複数出力対応Logger取得関数
 * @return writerとformaterを設定
 */
logger::Logger& get_logger() {
    static std::once_flag flag;
    static std::unique_ptr<logger::Logger> instance;
    static bool colored = true;

    std::call_once(flag, [&]() {
        // 複数の出力ペアを作成
        std::vector<logger::LoggerPair> pairs;

        // ペア1: コンソール出力（カラー有効）
        pairs.emplace_back(
            std::make_unique<logger::Formatters::ConsoleFmt>(colored),
            std::make_unique<logger::Writers::BufferedWriter>());

        // ペア2: ファイル出力（カラー無効）
        // pairs.emplace_back(
        //     std::make_unique<logger::Formatters::ConsoleFmt>(false),
        //     std::make_unique<logger::Writers::FileWriter>("application.log"));

        // Loggerインスタンスを作成
        instance = std::make_unique<logger::Logger>(std::move(pairs));
        instance->set_level(LogLevel::INFO_);

        instance->log_output(LogLevel::INFO_, __FILE__, __LINE__,
                             "Logger initialized with %d output destinations.",
                             instance->get_output_count());

        instance->log_output(LogLevel::INFO_, __FILE__, __LINE__,
                             "Color output is %s. And color check is %s ",
                             colored ? "g|enabled|" : "disabled",
                             COL_CHECK ? "g|enabled|" : "disabled");
    });

    return *instance;
}

// ログ出力マクロ (カラータグ検証統合)
#if COL_CHECK
#define LOG_OUTPUT(level, fmt, ...)                                         \
    do {                                                                    \
        static_assert(logger::Utils::ValidationUtils::check_colors_ct(fmt), \
                      "Invalid color tags");                                \
        get_logger().log_output(level, __FILE__, __LINE__, fmt,             \
                                ##__VA_ARGS__);                             \
    } while (0)
#else
#define LOG_OUTPUT(level, fmt, ...) \
    get_logger().log_output(level, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

// ログレベル別マクロ
#define LOG_DEBUG(fmt, ...) LOG_OUTPUT(LogLevel::DEBUG_, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_OUTPUT(LogLevel::INFO_, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_OUTPUT(LogLevel::WARN_, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_OUTPUT(LogLevel::ERROR_, fmt, ##__VA_ARGS__)

#define FLUSH_BUFF() get_logger().flush()

#endif  // LOGGER_HPP