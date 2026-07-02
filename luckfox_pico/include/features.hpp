#ifndef FEATURES_HPP
#define FEATURES_HPP

#include <string>
#include <unordered_map>
#include <cmath>
#include <cctype>

inline float digit_ratio(const std::string &s)
{
    if (s.empty())
        return 0.0;
    size_t count = 0;
    for (unsigned char ch : s) if (std::isdigit(ch)) ++count;
    return static_cast<float>(count) / s.size();
}

inline float shannon_entropy(const std::string &s)
{
    if (s.empty())
        return 0.0;
    std::unordered_map<unsigned char, size_t> freq;
    for (unsigned char ch : s) ++freq[ch];
    const float n = static_cast<float>(s.size());
    float ent = 0.0;
    for (const auto &p : freq)
    {
        float f = static_cast<float>(p.second) / n;
        ent -= f * std::log2(f);
    }
    return ent;
}

inline size_t length(const std::string &s)
{
    return s.size();
}

inline float uppercase_ratio(const std::string &s)
{
    if (s.empty())
        return 0.0;
    size_t count = 0;
    for (unsigned char ch : s) if (std::isupper(ch)) ++count;
    return static_cast<float>(count) / s.size();
}
#endif
