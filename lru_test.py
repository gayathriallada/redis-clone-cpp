import socket

def send(s, cmd):
    s.sendall((cmd + "\n").encode())
    return s.recv(1024).decode().strip()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("127.0.0.1", 6380))

print("SET a:", send(s, "SET a 1"))
print("SET b:", send(s, "SET b 2"))
print("GET a (touch it, so 'a' becomes most-recently-used):", send(s, "GET a"))
print("SET c (capacity is 2, so this should evict 'b', the least-recently-used):", send(s, "SET c 3"))
print("GET a (should still exist):", send(s, "GET a"))
print("GET b (should be GONE - evicted):", send(s, "GET b"))
print("GET c (should exist):", send(s, "GET c"))

s.close()
