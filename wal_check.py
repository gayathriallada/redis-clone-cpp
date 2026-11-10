import socket, sys, time
s = socket.create_connection(("127.0.0.1", 6380))
for cmd in sys.argv[1:]:
    s.sendall((cmd + "\n").encode())
    time.sleep(0.2)
    print(cmd, "->", s.recv(1024).decode().strip())
