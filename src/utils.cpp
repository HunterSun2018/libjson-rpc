#ifdef BOOST 
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#endif
#include <iomanip>
#include <sstream>
#include "utils.h"

using namespace std;

namespace utils
{
#ifdef BOOST    
std::string encode64(const std::vector<u_char> &val)
{
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::vector<u_char>::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

std::string encode64(const u_char *buffer, uint len)
{
    return encode64(std::vector<u_char>(buffer, buffer + len));
}

std::vector<u_char> decode64(const std::string &val)
{
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::vector<u_char>(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
    });
}
#endif

std::string hex(u_char *buffer, uint length)
{
    std::ostringstream oss;

    for (uint i = 0; i < length; i++)
        oss << std::setfill('0') << std::setw(2) << std::hex << (int)buffer[i];

    return oss.str();
}

std::vector<u_char> from_hex(std::string str)
{
    std::vector<u_char> buffer;

    for (size_t i = 0; i < str.size(); i+=2)
    {
        std::istringstream iss(str.substr(i, 2));
        uint byte;

        iss >> std::hex >> std::setw(2) >> byte; // std::setfill('0') >> std::setw(2)
        buffer.push_back(byte);
    }

    return buffer;
}

std::string replace(const std::string &str, const std::string &from, const std::string &to)
{
    std::string temp = str;

    size_t start_pos = temp.find(from);
    if (start_pos != std::string::npos)
        temp.replace(start_pos, from.length(), to);

    return temp;
}
} // namespace utils
