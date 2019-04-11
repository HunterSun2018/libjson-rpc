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

  template <typename... T>
  auto call(uint32_t id, const std::string &method, T... args);

  struct status
  {
    int32_t error;
    std::string message;
  };

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

  template <typename TFunc>
  void register_notify(std::string id, TFunc func)
  {

  }

protected:
  virtual std::string run(const std:: string & request) = 0;

  typedef std::function<void(const status &s, Json)> callback;
  virtual void async_remote_call(json_request & request, callback cb) = 0;
};

typedef std::shared_ptr<client> client_ptr;

template <typename... T>
auto client::call(uint32_t id, const std::string &method, T... args)
{

  json_request js(id, method);

  ((js << args), ...);

  std::string response = run(js.to_string());

  std::istringstream iss(response);
  //arg2_type ret;
  int ret;

  iss >> ret;

  return "";
}

template <class Tuple, typename F>
void client::async_call(uint32_t id, const std::string &method, Tuple &&t, F f)
{
  json_request request(id, method);
  
  //
  //  serialize all arguments to JSON stream
  //
  std::apply([&](auto &... args) { ((request << args), ...); }, t);

  async_remote_call(request, [&](const status &s, Json result) {
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
    arg2_type ret = result.get<arg2_type>();

    func(s, ret);
  });
}

} // namespace rpc

#endif