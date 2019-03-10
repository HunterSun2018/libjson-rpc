#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H
#include "std.hpp"
#include "serialization.hpp"

namespace rpc
{
class client //: public client_base
{
public:
  static std::shared_ptr<client> create();

  virtual ~client() {}

  template <typename T>
  struct response
  {
    //as
  };

  virtual void connect(const std::string &ip, unsigned short port) = 0;

  virtual void async_connect(const std::string &ip, unsigned short port) = 0;

  template <typename... T>
  auto call(uint32_t id, const std::string &method, T... args)
  {
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

protected:
private:
};

typedef std::shared_ptr<client> client_ptr;

} // namespace rpc

#endif