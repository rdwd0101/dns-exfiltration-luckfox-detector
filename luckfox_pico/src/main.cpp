#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <chrono>
#include <array>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include "nn_model.hpp"
#include "constants.hpp"
#include "../../common/include/utils.hpp"
#include "../../common/include/constants.hpp"

namespace utils
{
    std::shared_ptr<spdlog::logger> configure_logger()
    {
        auto dns_queries_logger = spdlog::rotating_logger_mt(
            dns_exfiltration_detector::luckfox::constants::SPDLOG_DNS_QUERIES_LOGGER_NAME,
            dns_exfiltration_detector::luckfox::constants::SPDLOG_DNS_QUERIES_LOGGER_FILENAME,
            dns_exfiltration_detector::luckfox::constants::SPDLOG_FILE_SIZE_MAX,
            dns_exfiltration_detector::luckfox::constants::SPDLOG_FILES_COUNT
        );
        dns_queries_logger->set_level(spdlog::level::debug);
        spdlog::flush_every(std::chrono::seconds(1));
        return dns_queries_logger;
    }
}

int main(int argc, char** argv)
{
    auto dns_queries_logger = utils::configure_logger();
    
    //
    // init RKNN classifier for inference
    //
    dns_queries_logger->info("Daemon started");
    auto classifier = dns_exfiltration_detector::model::DNSClassifier(dns_exfiltration_detector::luckfox::constants::CLASSIFIER_MODEL_PATH);

    int local_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_fd < 0)
    {
        spdlog::critical("Failed to create local socket");
        return 1;
    }

    sockaddr_in local_addr{};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(dns_exfiltration_detector::common::constants::DNS_PORT);

    if (bind(local_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
    {
        spdlog::critical("Bind failed. Try to run as root (sudo) to use port 53");
        close(local_fd);
        return 1;
    }

    int upstream_fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in upstream_addr{};
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_addr.s_addr = inet_addr(dns_exfiltration_detector::common::constants::UPSTREAM_DNS);
    upstream_addr.sin_port = htons(dns_exfiltration_detector::common::constants::DNS_PORT);

    spdlog::info("DNS Proxy listening on port {}...", upstream_addr.sin_port);

    unsigned char buffer[dns_exfiltration_detector::common::constants::DNS_QUERY_BUFFER_SIZE];

    while (true)
    {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        size_t req_len = recvfrom(local_fd, buffer, dns_exfiltration_detector::common::constants::DNS_QUERY_BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);        
        
        if (req_len < dns_exfiltration_detector::common::constants::DNS_QUERY_MINIMUM_LENGTH)
        {
            spdlog::error("A valid DNS header must be at least {} bytes, got: {}", dns_exfiltration_detector::common::constants::DNS_QUERY_MINIMUM_LENGTH, req_len);
            continue;
        }
        
        std::string dns_name;
        dns_exfiltration_detector::common::utils::parse_dns_name(buffer, dns_name);
        
        spdlog::debug("Got query: {}", dns_name);

        //
        // Perform inference
        //
        bool is_exfiltration = classifier.run(dns_name);
        if (is_exfiltration)
        {
            dns_queries_logger->warn("Warning: classifier detected possible exfiltration, query: {}", dns_name);
            spdlog::warn("Warning: classifier detected possible exfiltration, query: {}", dns_name);
            continue;
        }
        else
        {
            spdlog::info("Classified query {} as a legitimate, proceeding to send DNS request to upstream...", dns_name);
            continue;
        }

        sendto(upstream_fd, buffer, req_len, 0, (struct sockaddr*)&upstream_addr, sizeof(upstream_addr));

        //
        // Receive response from upstream
        //
        size_t res_len = recvfrom(upstream_fd, buffer, dns_exfiltration_detector::common::constants::DNS_QUERY_BUFFER_SIZE, 0, nullptr, nullptr);
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
