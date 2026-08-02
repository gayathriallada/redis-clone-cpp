import socket

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("127.0.0.1", 6380))

def send(cmd):
    s.sendall((cmd + "\n").encode())
    response = s.recv(1024).decode()
    print(f">> {cmd}")
    print(f"<< {response.strip()}")

send("SET name gayathri")
send("GET name")
send("GET missing")
send("DEL name")
send("GET name")

s.close()
