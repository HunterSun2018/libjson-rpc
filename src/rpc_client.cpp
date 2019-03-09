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

  client_imp(std::string ip, unsigned short port)
  {
    connect(ip, port);
  }

  void connect(std::string ip, unsigned short port)
  {
    //tcp::endpoint endpoint(ip, port);
  }

  void async_connect(std::string ip, unsigned short port)
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

std::shared_ptr<client> client::create(std::string ip, unsigned short port)
{
  return make_shared<client_imp>(ip, port);
}

} // namespace rpc