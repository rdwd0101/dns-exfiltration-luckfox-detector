#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <chrono>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "nn_model.hpp"

#define DNS_PORT 53
#define BUFFER_SIZE 1024
#define DNS_QUERY_MINIMUM_LENGTH 12

const auto max_log_size = 1048576 * 5;
const auto max_log_files = 3;

const char* UPSTREAM_DNS = "8.8.8.8";
const char* MODEL_PATH = "model.rknn";

void parse_dns_name(unsigned char* buffer, int header_offset, char* out_domain)
{
    int i = header_offset;
    int out_index = 0;

    while (buffer[i] != 0)
    {
        int label_len = buffer[i];

        for (int j = 0; j < label_len; ++j)
        {
            out_domain[out_index++] = buffer[i++];
        }

        if (buffer[i] != 0)
        {
            out_domain[out_index++] = '.';
        }
        
        i++;
    }
    
    out_domain[out_index] = '\0';
}

int main()
{
    //
    // configure logs
    //
    auto dns_queries_logger = spdlog::rotating_logger_mt("dns_queries_logger", "dns_queries_log.txt", max_log_size, max_log_files);
    dns_queries_logger->set_level(spdlog::level::debug);
    spdlog::flush_every(std::chrono::seconds(1));
    //
    // init RKNN classifier for inference
    //
    dns_queries_logger->info("Daemon started");
    model::DNSClassifier classifier = model::DNSClassifier(MODEL_PATH);

    int local_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_fd < 0)
    {
        spdlog::critical("Failed to create local socket");
        return 1;
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(DNS_PORT);

    if (bind(local_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
    {
        spdlog::critical("Bind failed. Try to run as root (sudo) to use port 53");
        close(local_fd);
        return 1;
    }

    int upstream_fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in upstream_addr{};
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_addr.s_addr = inet_addr(UPSTREAM_DNS);
    upstream_addr.sin_port = htons(DNS_PORT);

    spdlog::info("DNS Proxy listening on port {}...", DNS_PORT);

    unsigned char buffer[BUFFER_SIZE];
    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        ssize_t req_len = recvfrom(local_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);        
        
        if (req_len < DNS_QUERY_MINIMUM_LENGTH)
        {
            spdlog::error("A valid DNS header must be at least {} bytes, got: {}", DNS_QUERY_MINIMUM_LENGTH, req_len);
            continue;
        }

        int dns_header_len = 12;
        char domain_name[256]; // Buffer to store the readable domain

        parse_dns_name(buffer, dns_header_len, domain_name);

        //std::string dns_query(domain_name);
        std::string dns_query(domain_name);
        spdlog::debug("Got query: {}", dns_query);

        //
        // Perform inference
        //
        bool is_exfiltration = classifier.run(dns_query);
        if (is_exfiltration)
        {
            dns_queries_logger->warn("Warning: classifier detected possible exfiltration, query: {}", dns_query);
            spdlog::warn("Warning: classifier detected possible exfiltration, query: {}", dns_query);
            continue;
        }
        else
        {
            spdlog::info("Classified query {} as a legitimate, proceeding to send DNS request to upstream...", dns_query);
            continue;
        }

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
