#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H
#include "rpc_server_base.hpp"
#include <string>
//#include <memory>

namespace rpc
{
class client : public client_base
{
  public:
    virtual ~client() {}

    virtual void connect(std::string ip, unsigned short port) = 0;

    virtual void async_connect(std::string ip, unsigned short port) = 0;

    static std::shared_ptr<client> create();
  protected:
  private:
};

typedef std::shared_ptr<client> client_ptr;

} // namespace rpc

#endif