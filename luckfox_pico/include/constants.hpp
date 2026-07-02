#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <stdlib.h>

namespace dns_exfiltration_detector
{
    namespace constants
    {
        constexpr size_t DNS_PORT = 53;
        constexpr size_t DNS_QUERY_BUFFER_SIZE = 1024;
        constexpr size_t DNS_DOMAIN_NAME_SIZE = 256;
        constexpr size_t DNS_QUERY_MINIMUM_LENGTH = 12;

        constexpr size_t SPDLOG_FILE_SIZE_MAX = 1048576 * 5;
        constexpr size_t SPDLOG_FILES_COUNT = 3;
        
        const char* SPDLOG_DNS_QUERIES_LOGGER_NAME = "dns_queries_logger";
        const char* SPDLOG_DNS_QUERIES_LOGGER_FILENAME = "dns_queries_log.txt";
        const char* UPSTREAM_DNS = "8.8.8.8";
        const char* CLASSIFIER_MODEL_PATH = "model.rknn";
    }
}
#endif
