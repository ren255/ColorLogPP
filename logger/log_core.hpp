#ifndef LOG_CORE_HPP
#define LOG_CORE_HPP
namespace logger {
/**
 * @brief Formatter/Writerペアを管理する構造体
 */
struct LoggerPair {
    std::unique_ptr<Formatters::FormatterBase> formatter;
    std::unique_ptr<Writers::IWriter> writer;

    LoggerPair(std::unique_ptr<Formatters::FormatterBase> fmt,
               std::unique_ptr<Writers::IWriter> wrt)
        : formatter(std::move(fmt)), writer(std::move(wrt)) {
        if (!formatter || !writer) {
            while (1) {
                printf("[ERROR_] formatterとwriteを設定して下さい\r\n");
            }
        }
    }
};

/**
 * @brief メインLoggerクラス
 * @details ログ出力の統括管理を行うオーケストレータ（複数出力対応）
 */
class Logger {
   private:
    LogLevel current_level;
    std::vector<LoggerPair> output_pairs;

    /**
     * @brief LogEntryを作成
     */
    LogEntry create_log_entry(LogLevel level, const char* file, int line,
                              const char* message) {
        LogEntry entry;
        entry.level = level;
        entry.filename = file;
        entry.line = line;
        entry.message = message;
        entry.formatedMsg = (char*)malloc(LOG_FMT_SIZE);
        return entry;
    }

    /**
     * @brief 内部ログ出力処理（全出力先に対して実行）
     */
    void log_internal(LogLevel level, const char* file, int line,
                      const char* message) {
        if (level < current_level) {
            return;
        }
        LogEntry entry;

        // 実行時バリデーション
        if (!Utils::ValidationUtils::validate_color_tags_runtime(message)) {
            entry = create_log_entry(LogLevel::ERROR_, file, line,
                                     "Invalid color tags");
        } else {
            entry = create_log_entry(level, file, line, message);
        }

        for (auto& pair : output_pairs) {
            pair.formatter->format(entry);
            pair.writer->write(entry);
        }

        free(entry.formatedMsg);  // バッファ解放
    }

   public:
    /**
     * @brief 複数出力ペア対応コンストラクタ
     * @param pairs 出力ペアのベクター
     */
    Logger(std::vector<LoggerPair> pairs)
        : current_level(LogLevel::INFO_), output_pairs(std::move(pairs)) {}

    /**
     * @brief 単一ペア用コンストラクタ
     */
    Logger(std::unique_ptr<Formatters::FormatterBase> fmt,
           std::unique_ptr<Writers::IWriter> wrt)
        : current_level(LogLevel::INFO_) {
        output_pairs.emplace_back(std::move(fmt), std::move(wrt));
    }

    // 可変引数処理用ヘルパー関数
    void log_output(LogLevel level, const char* file, int line, const char* fmt,
                    ...) {
        va_list args;
        va_start(args, fmt);
        char msg[256];
        vsnprintf(msg, sizeof(msg), fmt, args);
        va_end(args);
        log_internal(level, file, line, msg);
    }

    void flush() {
        for (auto& pair : output_pairs) {
            pair.writer->flush();
        }
    }

    /**
     * @brief 最小ログレベルを設定
     * @param level 設定するログレベル
     */
    void set_level(LogLevel level) { current_level = level; }

    /**
     * @brief 現在のログレベルを取得
     * @return 現在のログレベル
     */
    LogLevel get_level() const { return current_level; }

    /**
     * @brief 出力ペア数を取得
     * @return 設定されている出力ペアの数
     */
    size_t get_output_count() const { return output_pairs.size(); }
};

}  // namespace logger
#endif  // LOG_CORE_HPP