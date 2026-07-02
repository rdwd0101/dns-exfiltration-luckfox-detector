#include <gtest/gtest.h>

#include "../common/include/utils.hpp"
#include "../common/include/constants.hpp"

TEST(UtilsTests, WhenEmptyOutString_ThrowsError)
{
    std::array<unsigned char, dns_exfiltration_detector::common::constants::DNS_QUERY_BUFFER_SIZE> dns_query { 0 };
    
    std::string result;
    EXPECT_THROW(dns_exfiltration_detector::common::utils::parse_dns_name(dns_query.data(), result), std::runtime_error);
}