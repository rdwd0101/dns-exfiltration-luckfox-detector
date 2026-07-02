#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "constants.hpp"

namespace dns_exfiltration_detector
{
    namespace common
    {
        namespace utils
        {
            void parse_dns_name(unsigned char* buffer, std::string& domain_out)
            {
                if (domain_out.size() < dns_exfiltration_detector::common::constants::DNS_DOMAIN_NAME_SIZE)
                {
                    throw std::runtime_error("Invalid out buffer for DNS name");
                }
                if (buffer == nullptr)
                {
                    throw std::runtime_error("Invalid in buffer");
                }

                size_t i = dns_exfiltration_detector::common::constants::DNS_QUERY_MINIMUM_LENGTH;
                size_t out_index = 0;

                while (buffer[i] != 0)
                {
                    size_t label_len = buffer[i];

                    for (size_t j = 0; j < label_len; ++j)
                    {
                        domain_out[out_index++] = buffer[i++];
                    }

                    if (buffer[i] != 0)
                    {
                        domain_out[out_index++] = '.';
                    }
                    
                    i++;
                }
                
                domain_out[out_index + 1] = '\0';
            }

            /* TODO: merge two:
            void parse_dns_name_rpi(unsigned char* buffer, std::string& domain_out)
            {
                char domain[dns_exfiltration_detector::common::constants::DNS_DOMAIN_NAME_SIZE] = {0};
                size_t idx = dns_exfiltration_detector::common::constants::DNS_QUERY_MINIMUM_LENGTH;
                size_t dest_idx = 0;
                
                while (idx < dns_exfiltration_detector::common::constants::DNS_DOMAIN_NAME_SIZE && buffer[idx] != 0 && dest_idx < 254)
                {
                    int label_len = buffer[idx];
                    idx++;
                    
                    for (int i = 0; i < label_len && idx < dns_exfiltration_detector::common::constants::DNS_DOMAIN_NAME_SIZE; i++)
                    {
                        domain[dest_idx++] = buffer[idx++];
                    }
                    
                    domain[dest_idx++] = '.';
                }
                domain[dest_idx] = '\0';
                domain_out = std::string(domain);
            }

            void parse_dns_name_luckfox(unsigned char* buffer, std::string& domain_out)
            {
                if (domain_out.size() < dns_exfiltration_detector::common::constants::DNS_DOMAIN_NAME_SIZE)
                {
                    throw std::runtime_error("Invalid out buffer for DNS name");
                }
                if (buffer == nullptr)
                {
                    throw std::runtime_error("Invalid in buffer");
                }

                size_t i = dns_exfiltration_detector::common::constants::DNS_QUERY_MINIMUM_LENGTH;
                size_t out_index = 0;

                while (buffer[i] != 0)
                {
                    size_t label_len = buffer[i];

                    for (size_t j = 0; j < label_len; ++j)
                    {
                        domain_out[out_index++] = buffer[i++];
                    }

                    if (buffer[i] != 0)
                    {
                        domain_out[out_index++] = '.';
                    }
                    
                    i++;
                }
                
                domain_out[out_index + 1] = '\0';
            }
            */
        }
    }
}


#endif