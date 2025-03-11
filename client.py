import socket
import sys
import threading

class ClientStats:
    def __init__(self):
        self.bytes_sent = 0
        self.bytes_received = 0

def receive_messages(client_socket, stats):
    while True:
        data = client_socket.recv(1500)  # Match server buffer size
        
        if not data:
            print("\n[INFO] Server has disconnected.")
            break  # Exit the loop when server disconnects
        
        stats.bytes_received += len(data)
        print(f"Server: {data.decode().strip()}")

    # Close the connection
    client_socket.close()
    print("Connection closed")
    print("********Session Summary********")
    print(f"Bytes written: {stats.bytes_sent} Bytes read: {stats.bytes_received}")
    
            
    

def main():
    if len(sys.argv) != 3:
        print("Usage: python client.py <server_ip> <port>")
        sys.exit(1)

    server_ip = sys.argv[1]
    port = int(sys.argv[2])
    
    stats = ClientStats()
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        print(f"Attempting to connect to {server_ip}:{port}...")
        client_socket.connect((server_ip, port))
        print("Connected to server!")

        # Start listening for messages in a separate thread
        receive_thread = threading.Thread(target=receive_messages, args=(client_socket, stats))
        receive_thread.daemon = True  # Auto-exit when main program exits
        receive_thread.start()

        # Sending loop (User input)
        while True:
            message = input("> ")
            if message.lower() == "exit":
                break
                
            encoded_message = message.encode()
            client_socket.sendall(encoded_message)
            stats.bytes_sent += len(encoded_message)

    except ConnectionRefusedError:
        print("Failed to connect to the server. Make sure it's running and the port is correct.")
    except Exception as e:
        print(f"An error occurred: {e}")

    finally:
        # Close the connection
        client_socket.close()
        print("Connection closed")
        print("********Session Summary********")
        print(f"Bytes written: {stats.bytes_sent} Bytes read: {stats.bytes_received}")

if __name__ == "__main__":
    main()