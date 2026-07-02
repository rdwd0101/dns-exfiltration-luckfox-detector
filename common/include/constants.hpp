#ifndef COMMON_CONSTANTS_HPP
#define COMMON_CONSTANTS_HPP
#include <stdlib.h>

namespace dns_exfiltration_detector
{
    namespace common
    {
        namespace constants
        {
            constexpr size_t DNS_PORT = 53;
            constexpr size_t DNS_QUERY_BUFFER_SIZE = 1024;
            constexpr size_t DNS_DOMAIN_NAME_SIZE = 256;
            constexpr size_t DNS_QUERY_MINIMUM_LENGTH = 12;

            const char* UPSTREAM_DNS = "8.8.8.8";
        }
    }
}
#endif
