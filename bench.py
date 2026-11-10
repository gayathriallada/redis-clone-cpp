import socket, time, statistics

N = 20000
s = socket.create_connection(("127.0.0.1", 6380))
s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

def run(label, make_cmd):
    lat = []
    start = time.perf_counter()
    for i in range(N):
        t0 = time.perf_counter()
        s.sendall(make_cmd(i).encode())
        s.recv(1024)
        lat.append((time.perf_counter() - t0) * 1000)
    total = time.perf_counter() - start
    lat.sort()
    print(f"{label}: {N/total:.0f} req/sec | avg {statistics.mean(lat):.3f} ms | p50 {lat[N//2]:.3f} ms | p99 {lat[int(N*0.99)]:.3f} ms")

run("SET", lambda i: f"SET key{i} value{i}\n")
run("GET", lambda i: f"GET key{i}\n")
