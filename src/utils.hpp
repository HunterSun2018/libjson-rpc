#ifndef UTILS_H
#define UTILS_H
#include "std.hpp"
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>

namespace utils
{
#ifdef BOOST
std::string encode64(const std::vector<u_char> &val);
std::string encode64(const u_char *buffer, uint len);
#endif

std::vector<u_char> decode64(const std::string &val);

std::string hex(u_char *buffer, uint length);

std::vector<u_char> from_hex(std::string str);

std::string replace(const std::string &str, const std::string &from, const std::string &to);

template <typename... Args>
std::string format(const std::string &format, Args... args)
{
    size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

namespace detail
{
template <typename>
struct function_meta;

template <typename R, typename... T>
struct function_meta<std::function<R(T...)>>
{
    using return_type = std::decay_t<R>;
    using arguments_tuple_type = std::tuple<std::decay_t<T>...>;
};
} // namespace detail

template <typename T>
class Singleton
{
public:
    Singleton() = delete;

    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

    static T &instance()
    {
        static T instance;
        return instance;
    }
};

typedef Singleton<boost::asio::io_service> g_io_service;

} // namespace utils

#define CHECK_WALLY_RET(r) \
    if (r != WALLY_OK)     \
        throw runtime_error(utils::format("wally function erro at line %d in function %s", __LINE__, __func__));

#define THROW_ERROR(x) throw runtime_error(utils::format("%s, %s : %d in %s", x, __func__, __LINE__, __FILE__));

#define DEBUGGING_STRING utils::format("%s : %d in %s", __func__, __LINE__, __FILE__)

#endif