import paramiko

def ssh_connect(host='127.0.0.1', port=2222, username='adversary', password='1234'):
    try:
        # Create SSH client
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())  # accept unknown keys

        # Connect to the server
        client.connect(hostname=host, port=port, username=username, password=password)
        print(f"Connected to {host}:{port} as {username}")


        print("Command output:", stdout.read().decode().strip())

    except paramiko.AuthenticationException:
        print("Authentication failed")
    except paramiko.SSHException as e:
        print(f"SSH Error: {e}")
    finally:
        client.close()
        print("Connection closed")

if __name__ == "__main__":
    ssh_connect()