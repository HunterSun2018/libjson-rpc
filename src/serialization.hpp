#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP
#include "jsonrp.hpp"

template <typename>
struct is_pair : std::false_type
{
};

template <typename T, typename U>
struct is_pair<std::pair<T, U>> : std::true_type
{
};

template <class ITR, typename = typename std::enable_if<is_pair<typename ITR::value_type>::value, ITR>::type>
decltype(auto) do_stuff(ITR &&itr)
{
    //access of itr->second ok.
}

namespace rpc
{
class json_stream
{
  public:
    json_stream(uint32_t id, const std::string &method) : m_request(id, method, Json())
    {
    }

    template <typename T>
    json_stream &operator<<(const T &t)
    {
        if constexpr (is_pair<T>::value)
            m_params[t.first] = t.second;
        else
            m_params.push_back(t);

        return *this;
    }

    std::string to_string()
    {
        m_request.params = m_params;
        return m_request.to_json().dump();
    }

  private:
    jsonrpcpp::Request m_request;
    Json m_params;
};
} // namespace rpc
#endif