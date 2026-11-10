import socket, subprocess, time

def connect():
    while True:
        try:
            return socket.create_connection(("127.0.0.1", 6380))
        except OSError:
            time.sleep(0.005)

srv = subprocess.Popen(["./server"], stdout=subprocess.DEVNULL)
s = connect()
for i in range(100000):
    s.sendall(f"SET key{i} value{i}\n".encode())
    s.recv(1024)
s.sendall(b"SAVE\n"); print("SAVE ->", s.recv(1024).decode().strip())
s.close()
srv.kill(); srv.wait()

t0 = time.perf_counter()
srv = subprocess.Popen(["./server"], stdout=subprocess.DEVNULL)
s = connect()
s.sendall(b"GET key99999\n")
print("GET key99999 ->", s.recv(1024).decode().strip())
print(f"Reload time (100k keys): {(time.perf_counter() - t0) * 1000:.0f} ms")
srv.kill()
