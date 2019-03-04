#include "rpc_client.hpp"
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

namespace rpc
{
class client_imp : public client_base
{
    public:
    virtual ~client_imp() { }

    virtual void connect(std::string ip, unsigned short port )
    {
        //tcp::endpoint endpoint(ip, port);

    }
    
};
}
