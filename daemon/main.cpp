#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "features.hpp"

#define DNS_PORT 53
#define BUFFER_SIZE 1024

const char* UPSTREAM_DNS = "8.8.8.8";

int main()
{
    int local_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_fd < 0)
    {
        std::cerr << "Failed to create local socket" << std::endl;
        return 1;
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(DNS_PORT);

    if (bind(local_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
    {
        std::cerr << "Bind failed. Try to run as root (sudo) to use port 53." << std::endl;
        close(local_fd);
        return 1;
    }

    int upstream_fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in upstream_addr{};
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_addr.s_addr = inet_addr(UPSTREAM_DNS);
    upstream_addr.sin_port = htons(DNS_PORT);

    std::cout << "DNS Proxy listening on port " << DNS_PORT << "..." << std::endl;

    uint8_t buffer[BUFFER_SIZE];
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        ssize_t req_len = recvfrom(local_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);

        if (req_len < 0)
            continue;

        sendto(upstream_fd, buffer, req_len, 0, (struct sockaddr*)&upstream_addr, sizeof(upstream_addr));

        //
        // Receive response from upstream
        //
        ssize_t res_len = recvfrom(upstream_fd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);
        if (res_len < 0)
            continue;

        //
        // Send response back to the client
        //
        sendto(local_fd, buffer, res_len, 0, (struct sockaddr*)&client_addr, client_len);
    }

    close(local_fd);
    close(upstream_fd);
    return 0;
}
