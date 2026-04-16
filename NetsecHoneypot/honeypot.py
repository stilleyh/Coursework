import socket
import threading
import paramiko
import logging
from datetime import datetime

 
HOST_KEY = paramiko.RSAKey.generate(2048)  # ephemeral server key

class SSHHoneypot(paramiko.ServerInterface):
    def __init__(self):
        self.event = threading.Event()

    def check_channel_request(self, kind, chanid):
        if kind == 'session':
            return paramiko.OPEN_SUCCEEDED
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    def check_auth_password(self, username, password):
        # Log credentials attempted
        print(f"[{datetime.now()}] Attempted login: {username}:{password}")
        return paramiko.AUTH_FAILED  # never allow login

def handle_client(client_socket, addr):
    try:
        transport = paramiko.Transport(client_socket)
        transport.add_server_key(HOST_KEY)
        server = SSHHoneypot()
        try:
            transport.start_server(server=server)
        except paramiko.SSHException:
            print(f"[{datetime.now()}] SSH negotiation failed from {addr}")
            return

        # Wait for a channel
        chan = transport.accept(20)
        if chan is not None:
            chan.close()
        transport.close()
    except Exception as e:
        print(f"[{datetime.now()}] Exception: {e}")
    finally:
        client_socket.close()
# main method
def start_honeypot(host='0.0.0.0', port=2222):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((host, port))
    server_socket.listen(100)
    server_socket.settimeout(10)
    print(f"[{datetime.now()}] SSH Honeypot listening on {host}:{port}")

    try:
        while True:
            client, addr = server_socket.accept()
            print(f"[{datetime.now()}] Connection from {addr}")
            t = threading.Thread(target=handle_client, args=(client, addr))
            t.start()
    except KeyboardInterrupt, TimeoutError:
        print(f"\nShutting down. [{datetime.now()}]")

if __name__ == "__main__":
    start_honeypot()