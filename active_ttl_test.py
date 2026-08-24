import socket, time

def connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("127.0.0.1", 6380))
    return s

def send(s, cmd):
    s.sendall((cmd + "\n").encode())
    return s.recv(1024).decode().strip()

# First connection: set a short-TTL key, then disconnect completely
s1 = connect()
print("SET with 2s TTL:", send(s1, "SET ghost boo EX 2"))
s1.close()

print("Waiting 4 seconds (server should sweep it away on its own)...")
time.sleep(4)

# A brand new connection - we never called GET during the wait
s2 = connect()
print("GET from a fresh connection (should be nil - swept by background check):", send(s2, "GET ghost"))
s2.close()
