#ifndef LOG_UTILS_HPP
#define LOG_UTILS_HPP

namespace logger {
/**
 * @brief ユーティリティ機能を提供する名前空間
 */
namespace Utils {

/**
 * @brief カラー処理統合クラス
 * @details カラータグの解析、ANSIコード変換などを一元管理
 */
class ColorHelper {
   public:
    /**
     * @brief ログレベルに応じたカラーコードを取得
     * @param level ログレベル
     * @param color_enabled カラー出力が有効か
     * @return ANSIカラーコード
     */
    static const char* get_level_color(LogLevel level, bool color_enabled) {
        if (!color_enabled) return "";

        auto it = ColorMap::LEVEL_COLORS.find(level);
        return (it != ColorMap::LEVEL_COLORS.end()) ? it->second : "";
    }

    /**
     * @brief カラーリセットコードを取得
     * @param color_enabled カラー出力が有効か
     * @return ANSIリセットコード
     */
    static const char* get_reset_color(bool color_enabled) {
        return color_enabled ? ColorMap::RESET : "";
    }

    /**
     * @brief カラータグ付きメッセージを解析してANSIコードに変換
     * @param input 入力メッセージ
     * @param output 出力バッファ
     * @param max_len 最大長
     * @param color_enabled カラー出力が有効か
     */
    static void parse_color_tags(const char* input, char* output, int max_len) {
        bool pipe_odd = false;  // falseなら偶数、開始タグ
        int out_pos = 0;
        int input_len = strlen(input);

        for (int i = 0; i < input_len && out_pos < max_len - 1; i++) {
            if (input[i] != '|') {
                // 通常文字
                output[out_pos++] = input[i];
                continue;
            }

            //  | があった:
            if (!pipe_odd) {  // 偶数、開始タグ
                // |連続はエスケープされたリテラル|として処理
                if (i + 1 < input_len && input[i + 1] == '|') {
                    output[out_pos++] = '|';  // リテラル|を出力
                    i++;                      // 次の|もスキップ
                    continue;
                }
                // カラータグ開始処理 (x|形式)
                auto it = ColorMap::ANSI_COLORS.find(input[i - 1]);
                if (it != ColorMap::ANSI_COLORS.end()) {
                    out_pos--;  // 色タグの文字を上書き
                    const char* color_code = it->second;
                    int code_len = strlen(color_code);
                    if (out_pos + code_len < max_len - 1) {
                        strcpy(output + out_pos, color_code);
                        out_pos += code_len;
                    }
                }
                pipe_odd = true;
                continue;
            } else {  // 奇数、終了タグ
                      // カラー終了タグ (単独の|)
                int reset_len = strlen(ColorMap::RESET);
                if (out_pos + reset_len < max_len - 1) {
                    strcpy(output + out_pos, ColorMap::RESET);
                    out_pos += reset_len;
                }
                pipe_odd = false;
            }
        }
        output[out_pos] = '\0';
    }

    /**
     * @brief メッセージからカラータグを除去
     * @param input 入力メッセージ
     * @param output 出力バッファ
     * @param max_len 最大長
     */
    static void strip_color_tags(const char* input, char* output, int max_len) {
        bool pipe_odd = false;  // falseなら偶数、開始タグ
        int out_pos = 0;
        int input_len = strlen(input);

        for (int i = 0; i < input_len && out_pos < max_len - 1; i++) {
            if (input[i] != '|') {
                // 通常文字
                output[out_pos++] = input[i];
                continue;
            }

            //  | があった:
            if (!pipe_odd) {  // 偶数、開始タグ
                // |連続はエスケープされたリテラル|として処理
                if (i + 1 < input_len && input[i + 1] == '|') {
                    output[out_pos++] = '|';  // リテラル|を出力
                    i++;                      // 次の|もスキップ
                    continue;
                }
                // カラータグ開始処理 (x|形式) - スキップ
                pipe_odd = true;
                continue;
            } else {  // 奇数、終了タグ
                // カラー終了タグ (単独の|) - スキップ
                pipe_odd = false;
            }
        }
        output[out_pos] = '\0';
    }
};

/**
 * @brief 検証処理統合クラス
 * @details カラータグなどの妥当性検証を統合
 */
class ValidationUtils {
   public:
    /**
     * @brief カラータグの妥当性をチェック（コンパイル時）
     * @param input チェック対象の文字列
     * @return true: 妥当, false: 不正
     * @details
     * - | が先頭でない
     * - 色タグが a-z の範囲内か（簡易チェック）
     * - | が偶数か
     */
    static constexpr bool check_colors_ct(const char* input) {
        bool pipe_odd = false;  // falseなら偶数、開始タグ
        int input_len = 0;

        // 文字列長を計算
        while (input[input_len] != '\0') {
            input_len++;
        }

        // 先頭が|でないかチェック
        if (input_len > 0 && input[0] == '|') {
            return false;
        }

        for (int i = 0; i < input_len; i++) {
            if (input[i] != '|') {
                continue;
            }

            //  | があった:
            if (!pipe_odd) {  // 偶数、開始タグ
                // |連続はスキップ
                if (i + 1 < input_len && input[i + 1] == '|') {
                    i++;
                    continue;
                }
                // 開始タグの前の色タグが正しいか（a-z の簡易チェック）
                if (input[i - 1] < 'a' || input[i - 1] > 'z') {
                    return false;
                }
                pipe_odd = true;
                continue;
            } else {  // 奇数、終了タグ
                pipe_odd = false;
            }
        }

        return !pipe_odd;  // 偶数ならtrue
    }

    /**
     * @brief カラータグの妥当性をチェック（実行時）
     * @details チェック事項
     * - | が先頭でない
     * - 色タグがColorMap::ANSI_COLORSに含まれるか
     * - | が偶数か
     * @param input チェック対象の文字列
     * @return true: 妥当, false: 不正
     */
    static bool validate_color_tags_runtime(const char* input) {
        bool pipe_odd = false;  // falseなら偶数、開始タグ
        int input_len = strlen(input);

        // 先頭が|でないかチェック
        if (input_len > 0 && input[0] == '|') {
            return false;
        }

        for (int i = 0; i < input_len; i++) {
            if (input[i] != '|') {
                continue;
            }

            //  | があった:
            if (!pipe_odd) {  // 偶数、開始タグ
                // |連続はスキップ
                if (i + 1 < input_len && input[i + 1] == '|') {
                    i++;
                    continue;
                }
                // 開始タグの前の色タグが正しいか
                if (ColorMap::ANSI_COLORS.count(input[i - 1]) == 0) {
                    return false;
                }
                pipe_odd = true;
                continue;
            } else {  // 奇数、終了タグ
                pipe_odd = false;
            }
        }

        if (!pipe_odd) {
            // 偶数ok
            return true;
        } else {
            // 奇数no
            return false;
        }
    }
};

/**
 * @brief 文字列処理統合クラス
 * @details 共通の文字列操作を統合
 */
class StringUtils {
   public:
    /**
     * @brief ファイルパスからファイル名のみを抽出
     * @param filepath フルパス
     * @return ファイル名部分
     */
    static const char* extract_filename(const char* filepath) {
        const char* filename = strrchr(filepath, '/');
        if (filename == nullptr) {
            filename = strrchr(filepath, '\\');
        }
        return (filename != nullptr) ? filename + 1 : filepath;
    }

    /**
     * @brief ログレベル文字列を取得
     * @param level ログレベル
     * @return レベル文字列
     */
    static const char* get_level_string(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG_:
                return "DEBUG";
            case LogLevel::INFO_:
                return "INFO";
            case LogLevel::WARN_:
                return "WARN";
            case LogLevel::ERROR_:
                return "ERROR";
            default:
                return "UNKNOWN";
        }
    }

    /**
     * @brief LogEntry専用snprintfラッパー
     * @details LogEntryのformatedMsgバッファに直接フォーマット出力
     * @param entry 出力先のLogEntry
     * @param format フォーマット文字列
     * @param ... 可変長引数
     * @return フォーマット済み文字数（バッファサイズ制限内）
     */
    static int log_sprintf(const LogEntry& entry, const char* format, ...) {
        va_list args;
        va_start(args, format);
        int result = vsnprintf(entry.formatedMsg, LOG_FMT_SIZE, format, args);
        va_end(args);
        return result;
    }

    /**
     * @brief 汎用パディング関数
     * @param text パディング対象文字列
     * @param target_width 目標幅
     * @param output 出力バッファ（target_width+1以上）
     * @param align_left 左揃え
     * @param pad_char パディング文字
     */
    static void apply_padding(const char* text, int target_width, char* output,
                              bool align_left = true, char pad_char = ' ') {
        int text_len = strlen(text);
        int padding = target_width - text_len;
        size_t buffer_size = target_width + 1;

        if (padding <= 0) {
            // パディング不要またはテキストが長すぎる場合
            snprintf(output, buffer_size, "%s", text);
            return;
        }

        if (align_left) {
            // 左揃え: "text    "
            snprintf(output, buffer_size, "%s%*s", text, padding, "");
        } else {
            // 右揃え: "    text"
            snprintf(output, buffer_size, "%*s%s", padding, "", text);
        }
    }

    /**
     * @brief 安全な文字列コピー
     * @param dest 宛先バッファ
     * @param src ソース文字列
     * @param max_len 最大長
     */
    static void safe_strcpy(char* dest, const char* src, int max_len) {
        strncpy(dest, src, max_len - 1);
        dest[max_len - 1] = '\0';
    }
};
}  // namespace Utils
}  // namespace logger

#endif  // LOG_UTILS_HPP