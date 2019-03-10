#include "rpc_client.hpp"
#include <boost/asio.hpp>
#include <memory>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

namespace rpc
{
class client_imp : public client
{
public:
  virtual ~client_imp() {}

  client_imp()
  {
  }

  virtual void connect(const std::string &ip, unsigned short port)
  {
    //tcp::endpoint endpoint(ip, port);
  }

  virtual void async_connect(const std::string &ip, unsigned short port)
  {
  }

protected:
  virtual void write(std::string request)
  {
    cout << request << endl;
  }

  virtual void async_write(std::string request)
  {
  }

  virtual std::string read()
  {
    return "";
  }

  virtual void async_read()
  {
  }
};

std::shared_ptr<client> client::create()
{
  return make_shared<client_imp>();
}

} // namespace rpc