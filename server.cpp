#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <vector>
#include <sstream>
#include <unordered_map>

std::unordered_map<std::string, std::string> store;
std::unordered_map<int, std::string> client_buffers;

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
        perror("Bind failed");
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

    std::vector<pollfd> poll_fds;

    pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);

    std::cout << "Entering event loop...\n";

    while (true) {
        int ready = poll(poll_fds.data(), poll_fds.size(), -1);
        if (ready == -1) {
            perror("poll failed");
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); i++) {
            if (poll_fds[i].revents & POLLIN) {

                if (poll_fds[i].fd == server_fd) {
                    int client_fd = accept(server_fd, nullptr, nullptr);
                    if (client_fd != -1) {
                        std::cout << "New client connected! fd = " << client_fd << "\n";
                        pollfd client_pollfd;
                        client_pollfd.fd = client_fd;
                        client_pollfd.events = POLLIN;
                        poll_fds.push_back(client_pollfd);
                    }
                } else {
                    char buffer[1024] = {0};
                    int bytes_read = recv(poll_fds[i].fd, buffer, sizeof(buffer), 0);

                    if (bytes_read <= 0) {
                        std::cout << "Client disconnected. fd = " << poll_fds[i].fd << "\n";
                        close(poll_fds[i].fd);
                        client_buffers.erase(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        i--;
                    } else {
                        client_buffers[poll_fds[i].fd].append(buffer, bytes_read);
                        std::string &clientBuf = client_buffers[poll_fds[i].fd];

                        size_t pos;
                        while ((pos = clientBuf.find('\n')) != std::string::npos) {
                            std::string line = clientBuf.substr(0, pos);
                            clientBuf.erase(0, pos + 1);

                            if (!line.empty() && line.back() == '\r') line.pop_back();

                            std::istringstream iss(line);
                            std::string cmd, key, value;
                            iss >> cmd >> key;
                            std::string response;

                            if (cmd.empty()) {
                                continue;
                            } else if (cmd == "SET") {
                                if (!(iss >> value)) {
                                    response = "ERROR: SET requires a key and a value\n";
                                } else {
                                    store[key] = value;
                                    response = "OK\n";
                                }
                            } else if (cmd == "GET") {
                                if (key.empty()) {
                                    response = "ERROR: GET requires a key\n";
                                } else if (store.count(key)) {
                                    response = store[key] + "\n";
                                } else {
                                    response = "(nil)\n";
                                }
                            } else if (cmd == "DEL") {
                                if (key.empty()) {
                                    response = "ERROR: DEL requires a key\n";
                                } else if (store.erase(key)) {
                                    response = "1\n";
                                } else {
                                    response = "0\n";
                                }
                            } else {
                                response = "ERROR: unknown command\n";
                            }

                            send(poll_fds[i].fd, response.c_str(), response.size(), 0);
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
