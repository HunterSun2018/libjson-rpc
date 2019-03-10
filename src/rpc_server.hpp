#ifndef STRATUM_SERVER_H
#define STRATUM_SERVER_H

#include "std.hpp"
//#include "rpc_base.hpp"

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

namespace rpc
{

struct server //: public server_base
{
    static std::shared_ptr<server> create(unsigned short port);

    virtual ~server(){};

    //server(const server& s) = delete;
    //server& operator=(const server& s) = delete;

    //server(const server&& s);
    //server& operator=(const server&& s);

    virtual bool run() = 0;

    virtual void stop() = 0;

    
    template <typename TFunc>
    void add_handler(std::string id, TFunc func)
    {
        if (m_handlers.find(id) != std::end(m_handlers))
        {
            throw std::invalid_argument{"[" + std::string{__func__} + "] Failed to add handler. "
                                                                      "The id \"" +
                                        id + "\" already exists."};
        }

        auto wrapper = [f = std::move(func)](std::string request) {
            std::function func{std::move(f)};

            using function_meta = detail::function_meta<decltype(func)>;
            using arguments_tuple_type = typename function_meta::arguments_tuple_type;

            arguments_tuple_type data;
            std::istringstream iss(request);

            std::apply([&](auto &... args) { ((iss >> args), ...); }, data);

            auto ret = std::apply(std::move(func), std::move(data));
            ret = decltype(ret)();
        };

        m_handlers.emplace(std::move(id), std::move(wrapper));
    }

    template <typename... T>
    auto call(std::string id, T... args)
    {
        std::ostringstream oss;

        ((oss << args << " "), ...);

        auto iter = m_handlers.find(id);

        if (iter != std::end(m_handlers))
        {
            return iter->second(oss.str());
        }
    }

    auto exec(const std::string &id, const std::string &serialization)
    {
        auto iter = m_handlers.find(id);

        if (iter != std::end(m_handlers))
        {
            return iter->second(serialization);
        }
    }

  private:
    typedef std::function<void(std::string)> handler_type;
    std::map<std::string, handler_type> m_handlers;
};

typedef std::shared_ptr<server> server_ptr;

} // namespace rpc

#endif