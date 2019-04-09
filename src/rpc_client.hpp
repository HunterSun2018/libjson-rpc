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

  typedef std::function<void(status)> connect_handler;
  typedef std::function<void(status)> write_handler;
  typedef std::function<void(status, const std::string &response)> read_handler;

  virtual void connect(const std::string &ip, unsigned short port) = 0;

  virtual void run() = 0;

  template <typename... T>
  auto call(uint32_t id, const std::string &method, T... args)
  {

    json_stream js(id, method);

    ((js << args), ...);

    write(js.to_string());

    read();

    return "";
  }

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
  void async_call(uint32_t id, const std::string &method, Tuple &&t, F f)
  {
    json_stream js(id, method);
    //
    //  serialize all arguments to JSON stream
    //
    std::apply([&](auto &... args) { ((js << args), ...); }, t);

    async_run(id, method, js, [&](status & s, std::string response) {
      std::function func{std::move(f)};

      using function_meta = utils::detail::function_meta<decltype(func)>;
      using arguments_tuple_type = typename function_meta::arguments_tuple_type;
      using arg2_type = typename std::tuple_element<1, arguments_tuple_type>::type;

      std::istringstream iss(response);
      arg2_type ret;

      iss >> ret;

      //
      //  deserialize response to JSON stream
      //
      func(s, ret);
    });
  }

protected:
  virtual void async_run(uint32_t id, const std::string &method, json_stream &stream, std::function<void(status & s, std::string)> f) = 0;

  virtual void async_connect(const std::string &ip, unsigned short port, connect_handler handler) = 0;
  /**
     *  send request to server
     */
  virtual void write(std::string request) = 0;

  /**
   *  send request to server
   */
  virtual void async_write(std::string request, write_handler &&handler) = 0;

  /**
     *  read the response from server
     */
  virtual std::string read() = 0;

  /**
   *  read the response from server
   */
  virtual void async_read(read_handler &&handler) = 0;

protected:
private:
};

typedef std::shared_ptr<client> client_ptr;

} // namespace rpc

#endif