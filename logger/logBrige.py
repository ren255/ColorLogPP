import time
import socket
import threading
import signal
import sys


class DataSender:
    """データ送信の基底クラス"""

    def __init__(self, mode='print'):
        self.mode = mode.lower()
        self.running = False

    def start(self):
        """送信開始"""
        self.running = True

    def stop(self):
        """送信停止"""
        self.running = False

    def send_data(self, data):
        """データ送信（サブクラスで実装）"""
        raise NotImplementedError


class PrintSender(DataSender):
    """コンソール出力モード"""

    def __init__(self):
        super().__init__('print')

    def start(self):
        super().start()
        print("Print Mode - コンソール出力開始 (Ctrl+Cで終了)")
        try:
            while self.running:
                data = self._generate_data()
                print(data.strip())
                time.sleep(1)
        except KeyboardInterrupt:
            self.stop()

    def _generate_data(self):
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
        return f"Python処理結果: Hello from Python!  time: {timestamp}"


class TelnetSender(DataSender):
    """Telnetサーバーモード"""

    def __init__(self, host='localhost', port=23):
        super().__init__('telnet')
        self.host = host
        self.port = port
        self.server_socket = None
        self.active_connections = []

    def start(self):
        super().start()
        try:
            self.server_socket = socket.socket(
                socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(
                socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)

            print(f"Telnet Mode - サーバー開始 {self.host}:{self.port} (Ctrl+Cで終了)")
            print("Tera Termで接続してください")

            signal.signal(signal.SIGINT, self._signal_handler)
            self._accept_connections()

        except Exception as e:
            print(f"Telnetサーバー起動エラー: {e}")
            self.stop()

    def stop(self):
        super().stop()
        print("\nTelnetサーバー終了処理中...")
        self._close_all_connections()
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass

    def _accept_connections(self):
        while self.running:
            try:
                self.server_socket.settimeout(1.0)
                conn, addr = self.server_socket.accept()
                thread = threading.Thread(
                    target=self._handle_client, args=(conn, addr))
                thread.daemon = True
                thread.start()
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"接続受付エラー: {e}")
                break

    def _handle_client(self, conn, addr):
        self.active_connections.append(conn)
        print(f"Telnetクライアント接続: {addr}")

        try:
            while self.running:
                data = self._generate_data()
                conn.send(data.encode())
                time.sleep(1)
        except (ConnectionResetError, BrokenPipeError):
            print(f"Telnetクライアント切断: {addr}")
        except Exception as e:
            print(f"Telnetクライアント処理エラー: {e}")
        finally:
            self._close_connection(conn)

    def _generate_data(self):
        timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
        return f"Python処理結果: Hello from Python!  time: {timestamp} \r\n"

    def _close_connection(self, conn):
        try:
            conn.close()
            if conn in self.active_connections:
                self.active_connections.remove(conn)
        except:
            pass

    def _close_all_connections(self):
        for conn in self.active_connections[:]:
            self._close_connection(conn)

    def _signal_handler(self, sig, frame):
        self.stop()
        sys.exit(0)


class CommunicationServer:
    """統合通信サーバー"""

    def __init__(self, mode='telnet+print', host='localhost', port=23):
        """
        Args:
            mode (str): 'print', 'telnet', または  デフォルト: telnet+print
            host (str): Telnetモード時のホスト名
            port (int): Telnetモード時のポート番号
        """
        self.mode = mode.lower()

        self.host = host
        self.port = port
        self.sender = None
        self.print_sender = None

    def create_sender(self):
        """モードに応じたSenderを作成"""
        if self.mode == 'print':
            self.sender = PrintSender()
        elif self.mode == 'telnet':
            self.sender = TelnetSender(self.host, self.port)
        elif self.mode == 'telnet+print':
            # Telnet + Print 両方のモード
            self.sender = TelnetSender(self.host, self.port)
            self.print_sender = PrintSender()
        else:
            raise ValueError(f"サポートされていないモード: {self.mode}")

        return self.sender

    def start(self):
        """サーバー開始"""
        if self.sender is None:
            self.create_sender()

        print(f"=== {self.mode.upper()} MODE ===")

        if self.mode == 'telnet+print':
            # Telnet + Print モード: 両方を並行実行
            print("Telnet + Print Mode - 両方同時実行")

            # Printを別スレッドで実行
            print_thread = threading.Thread(target=self._run_print_sender)
            print_thread.daemon = True
            print_thread.start()

            # Telnetをメインスレッドで実行
            self.sender.start()
        else:
            # 単一モード
            self.sender.start()

    def _run_print_sender(self):
        """Print Senderを実行（内部使用）"""
        if self.print_sender:
            self.print_sender.start()

    def stop(self):
        """サーバー停止"""
        if self.sender:
            self.sender.stop()
        if self.print_sender:
            self.print_sender.stop()


def main():
    """メイン関数 - 使用例"""
    demo_server = CommunicationServer()
    try:
        demo_server.start()
    except KeyboardInterrupt:
        demo_server.stop()
    except Exception as e:
        print(f"エラー: {e}")


if __name__ == "__main__":
    main()
