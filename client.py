import socket
import sys
import select
import threading

class ClientStats:
    def __init__(self):
        self.bytes_sent = 0
        self.bytes_received = 0

def receive_messages(client_socket,stats, disconnect):
    while not disconnect.is_set():
        try:
            response = client_socket.recv(1024).decode().strip()
            if not response:
                print("\n[INFO] Connection closed by server.")
                disconnect.set()
                break
            print(f"Server: {response}\n")
            stats.bytes_received += len(response)
        except (ConnectionResetError, OSError):
            print("\n[INFO] Connection lost while receiving.")
            disconnect.set()
            break

def start_client(host="localhost", port=12345):
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        client_socket.connect((host, port))
        print(f"Connecting to {host}:{port}...")
        print("Connected to server!")

        stats = ClientStats()
        disconnect = threading.Event()

        # Start a thread for receiving messages
        receive_thread = threading.Thread(target=receive_messages, args=(client_socket,stats, disconnect))
        receive_thread.daemon = True
        receive_thread.start()

        while not disconnect.is_set():
            rlist, _, _ = select.select([sys.stdin], [], [], 0.5)
            if disconnect.is_set():
                break

            if rlist:
                message = sys.stdin.readline().strip()
                if message.lower() == "exit":
                    print("Closing connection with server...")
                    disconnect.set()
                    break

                print(f"> {message}")  # Proper format for user input
                
                if disconnect.is_set() or client_socket.fileno() == -1:
                    print("\n[ERROR] Socket is closed, exiting...")
                    break

                try:
                    client_socket.sendall(message.encode())
                    stats.bytes_sent += len(message)
                except (BrokenPipeError, ConnectionResetError, OSError):
                    print("\n[INFO] Connection lost while sending.")
                    disconnect.set()
                    break

        client_socket.close()
        print("\nConnection closed")
        print("********Session Summary********")
        print(f"Bytes written: {stats.bytes_sent} Bytes read: {stats.bytes_received}")

    except ConnectionRefusedError:
        print("[ERROR] Unable to connect to the server.")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python client.py <host> <port>")
    else:
        start_client(sys.argv[1], int(sys.argv[2]))
