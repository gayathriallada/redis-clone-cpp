#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }
    std::cout << "Socket created successfully! fd = " << server_fd << "\n";

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(6380);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Bind successful on port 6380!\n";

    if (listen(server_fd, 5) == -1) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Listening on port 6380...\n";

    std::cout << "Waiting for a client to connect...\n";
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd == -1) {
        std::cerr << "Accept failed\n";
        close(server_fd);
        return 1;
    }
    std::cout << "Client connected! client_fd = " << client_fd << "\n";

    char buffer[1024] = {0};
    int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_read > 0) {
        std::cout << "Received: " << buffer << "\n";
        send(client_fd, buffer, bytes_read, 0);
        std::cout << "Echoed message back to client.\n";
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
