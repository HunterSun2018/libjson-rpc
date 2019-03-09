#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include "std.hpp"
#include "serialization.hpp"

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

class server_base
{
  public:
    virtual ~server_base() {}

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

class client_base
{
  public:
    virtual ~client_base() {}

    template <typename T>
    struct response
    {
    };

    template <typename... T>
    auto call(uint32_t id, const std::string &method, T... args)
    {
        //std::ostringstream oss;
        json_stream js(id, method);

        ((js << args), ...);

        write(js.to_string());

        read();

        return "";
    }

    template <class Tuple, typename Func>
    void async_call(uint32_t id, const std::string &method, Tuple &&t, Func &&f)
    {
        std::ostringstream oss;

        //((oss << args << " "), ...);
        std::apply([&](auto &... args) { ((oss << args << " "), ...); }, t);

        async_write(oss.str());

        async_read();
        //return std::make_pair(id, oss.str());

        f();
    }

  protected:
    /**
     *  send request to server
     */
    virtual void write(std::string request) = 0;

    /**
     *  send request to server
     */
    virtual void async_write(std::string request) = 0;

    /**
     *  read the response from server
     */
    virtual std::string read() = 0;

    /**
     *  read the response from server
     */
    virtual void async_read() = 0;
};

} // namespace rpc

#endif