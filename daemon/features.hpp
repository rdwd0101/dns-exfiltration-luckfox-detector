#include <string>
#include <unordered_map>
#include <cmath>
#include <cctype>

inline double digit_ratio(const std::string &s)
{
    if (s.empty())
        return 0.0;
    size_t count = 0;
    for (unsigned char ch : s) if (std::isdigit(ch)) ++count;
    return static_cast<double>(count) / s.size();
}

inline double shannon_entropy(const std::string &s)
{
    if (s.empty())
        return 0.0;
    std::unordered_map<unsigned char, size_t> freq;
    for (unsigned char ch : s) ++freq[ch];
    const double n = static_cast<double>(s.size());
    double ent = 0.0;
    for (const auto &p : freq)
    {
        double f = static_cast<double>(p.second) / n;
        ent -= f * std::log2(f);
    }
    return ent;
}

inline size_t length(const std::string &s)
{
    return s.size();
}

inline double uppercase_ratio(const std::string &s)
{
    if (s.empty())
        return 0.0;
    size_t count = 0;
    for (unsigned char ch : s) if (std::isupper(ch)) ++count;
    return static_cast<double>(count) / s.size();
}
