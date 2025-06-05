#ifndef LOG_WRITERS_HPP
#define LOG_WRITERS_HPP

#include <cstdio>

namespace logger {
/**
 * @brief 出力機能を提供する名前空間
 */
namespace Writers {

/**
 * @brief 出力インターフェース
 * @details 全ての出力先が実装すべき基底クラス
 */
class IWriter {
   public:
    virtual ~IWriter() = default;

    /**
     * @brief メッセージを出力
     * @param message 出力するメッセージ
     */
    virtual void write(const LogEntry& entry) = 0;

    /**
     * @brief BaseBufferedWriterの為
     */
    virtual void flush() = 0;
};

/**
 * @brief コンソール出力クラス
 * @details 標準出力へのメッセージ出力を担当
 */
class ConsoleWriter : public IWriter {
   public:
    /**
     * @brief コンソールにメッセージを出力
     * @param message 出力するメッセージ
     */
    void write(const LogEntry& entry) override {
        printf("%s\n", entry.formatedMsg);
    }
};

class DebugWriter : public IWriter {
   public:
    /**
     * @brief コンソールにメッセージを出力
     * @param message 出力するメッセージ
     */
    void write(const LogEntry& entry) override {
        // 通常print
        printf("%s\n", entry.formatedMsg);

        // rawプリント
        for (const char* p = entry.formatedMsg; *p; ++p) {
            if (*p == '\033') {
                printf("\\033");
            } else if (*p == '\n') {
                printf("\\n");
            } else if (*p == '\t') {
                printf("\\t");
            } else if (*p == '\r') {
                printf("\\r");
            } else {
                putchar(*p);
            }
        }

        // 改行
        printf("\n\n");
    }
};

/**
 * @brief バッファ付き出力基底クラス
 * @details メッセージをバッファリングして出力する基底クラス
 */
class BaseBufferedWriter : public IWriter {
   private:
    char buffer[BUFFER_SIZE];
    int buffer_pos = 0;

   public:
    /**
     * @brief コンストラクタ
     */
    BaseBufferedWriter() { buffer[0] = '\0'; }
    /**
     * @brief バッファの内容を取得
     * @return バッファの内容
     */
    const char* get_buffer() const { return buffer; }

    /**
     * @brief バッファをクリア
     */
    void clear_buffer() {
        buffer_pos = 0;
        buffer[0] = '\0';
    }

    /**
     * @brief バッファが空かどうかを確認
     * @return 空の場合true
     */
    bool is_empty() const { return buffer_pos == 0; }

    /**
     * @brief バッファにメッセージを保存
     * @param message 保存するメッセージ
     */
    void write(const LogEntry& entry) override {
        int msg_len = strlen(entry.formatedMsg);

        // バッファに余裕がない場合はフラッシュ
        if (buffer_pos + msg_len + 1 >= BUFFER_SIZE) {
            flush();
        }

        // バッファに追加
        strncpy(buffer + buffer_pos, entry.formatedMsg,
                BUFFER_SIZE - buffer_pos - 1);
        buffer_pos += msg_len;

        // バッファに改行を追加
        if (buffer_pos + 2 < BUFFER_SIZE) {
            buffer[buffer_pos++] = '\r';
            buffer[buffer_pos++] = '\n';
        }

        // 次のmsgで上書きされる。
        buffer[buffer_pos] = '\0';

        // // 改行文字が含まれていたらフラッシュ
        // if (strchr(entry.formatedMsg, '\n') != nullptr) {
        //     flush();
        // }
    }

    /**
     * @brief バッファの内容をクリア（基底クラスの処理）
     */
    virtual void flush() {
        // 基底クラスではバッファをクリアするのみ
        // 実際の出力は継承先で実装
        if (!is_empty()) {
            clear_buffer();
        }
    }

    /**
     * @brief デストラクタ - バッファをフラッシュ
     */
    virtual ~BaseBufferedWriter() { flush(); }
};

/**
 * @brief プレーンなバッファ出力クラス
 * @details BufferedWriterを継承してコンソール出力を実装
 */
class BufferedWriter : public BaseBufferedWriter {
   public:
    /**
     * @brief バッファの内容を出力してからクリア
     */
    void flush() override {
        if (!is_empty()) {
            // バッファの内容を出力
            printf("%s", get_buffer());
            // 親のflushを呼んでバッファをクリア
            BaseBufferedWriter::flush();
        }
    }
};

}  // namespace Writers
}  // namespace logger

#endif  // LOG_WRITERS_HPP