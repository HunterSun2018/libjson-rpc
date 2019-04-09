#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H
#include "std.hpp"
#include "utils.hpp"
#include "serialization.hpp"

namespace rpc
{

class client //: public client_base
{
public:
  static std::shared_ptr<client> create(std::string ip, uint16_t port);

  virtual ~client() {}

  struct status
  {
    int32_t error;
    std::string message;
  };

  template <typename... T>
  auto call(uint32_t id, const std::string &method, T... args);

  /**
   *  async call a RPC method
   *  id : context for caller
   *  method : a string name for method
   *  t : argument list with std::tuple type 
   *  f : callback funciotn whoes type is void(status s, const T & t)
   *  
   *  example :
   *      client->async_call(0, "add", make_tuple(1, 2), [](rpc::client::status s, int ret) {
   *         if (s.code)
   *         {
   *         }
   *      }); 
   */
  template <class Tuple, typename F>
  void async_call(uint32_t id, const std::string &method, Tuple &&t, F f);

protected:
  virtual std::string run(uint32_t id, const std::string &method, json_stream &stream) = 0;

  typedef std::function<void(const status &s, std::string)> callback;
  virtual void async_run(uint32_t id, const std::string &method, json_stream &stream, callback cb) = 0;
};

typedef std::shared_ptr<client> client_ptr;

template <typename... T>
auto client::call(uint32_t id, const std::string &method, T... args)
{

  json_stream js(id, method);

  ((js << args), ...);

  std::string response = run(id, method, js);

  std::istringstream iss(response);
  //arg2_type ret;
  int ret;

  iss >> ret;

  return "";
}

template <class Tuple, typename F>
void client::async_call(uint32_t id, const std::string &method, Tuple &&t, F f)
{
  json_stream js(id, method);
  //
  //  serialize all arguments to JSON stream
  //
  std::apply([&](auto &... args) { ((js << args), ...); }, t);

  async_run(id, method, js, [&](const status &s, std::string response) {
    // move callback to function object
    std::function func{std::move(f)};
    //
    // extract function meta data
    //
    using function_meta = utils::detail::function_meta<decltype(func)>;
    using arguments_tuple_type = typename function_meta::arguments_tuple_type;
    using arg2_type = typename std::tuple_element<1, arguments_tuple_type>::type;
    //
    //  deserialize response to JSON stream
    //
    std::istringstream iss(response);
    arg2_type ret;

    iss >> ret;
    //
    //  invoke callback function object
    //
    func(s, ret);
  });
}

} // namespace rpc

#endif