#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include <stdlib.h>

namespace dns_exfiltration_detector
{
    namespace luckfox
    {
        namespace constants
        {
            constexpr size_t SPDLOG_FILE_SIZE_MAX = 1048576 * 5;
            constexpr size_t SPDLOG_FILES_COUNT = 3;
            const char* SPDLOG_DNS_QUERIES_LOGGER_NAME = "dns_queries_logger";
            const char* SPDLOG_DNS_QUERIES_LOGGER_FILENAME = "dns_queries_log.txt";
            const char* CLASSIFIER_MODEL_PATH = "model.rknn";
        }
    }
}
#endif
