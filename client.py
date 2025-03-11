import socket
import sys

def main():
    # Check command line arguments
    if len(sys.argv) != 3:
        print("Usage: python client.py <server_ip> <port>")
        sys.exit(1)
    
    # Get server IP and port from command line
    server_ip = sys.argv[1]
    port = int(sys.argv[2])
    
    # Create socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    try:
        # Connect to the server
        print(f"Attempting to connect to {server_ip}:{port}...")
        client_socket.connect((server_ip, port))
        print("Connected to server!")
        
        # Track data sent and received
        bytes_written = 0
        bytes_read = 0
        
        # Main communication loop
        while True:
            # Get user input
            message = input("> ")
            
            # Send message to server
            client_socket.sendall(message.encode())
            bytes_written += len(message)
            
            # Check if client wants to exit
            if message == "exit":
                print("Closing connection with server...")
                break
            
            # Receive response from server
            buffer_size = 1500  # Same as server's msg array size
            data = client_socket.recv(buffer_size)
            bytes_read += len(data)
            
            # Decode and print the server's response
            server_message = data.decode().strip()
            print(f"Server: {server_message}")
            
            # Check if server wants to exit
            if server_message == "exit":
                print("Server has closed the connection")
                break
    
    except ConnectionRefusedError:
        print("Failed to connect to the server. Make sure it's running and the port is correct.")
    except Exception as e:
        print(f"An error occurred: {e}")
    
    finally:
        # Close the connection
        client_socket.close()
        print("Connection closed")
        print("********Session Summary********")
        print(f"Bytes written: {bytes_written} Bytes read: {bytes_read}")

if __name__ == "__main__":
    main()