#ifndef STRATUM_SERVER_H
#define STRATUM_SERVER_H

#include "std.hpp"
#include "rpc_server_base.hpp"

namespace rpc
{

struct server : public server_base
{
    virtual ~server(){};

    //server(const server& s) = delete;
    //server& operator=(const server& s) = delete;

    //server(const server&& s);
    //server& operator=(const server&& s);

    virtual bool init(unsigned short port,                 //server listening port
                      const std::string &network_node_url, //node rpc url
                      const std::string &user_pwd          //user:pwd
                      ) = 0;

    virtual bool run() = 0;

    virtual void stop() = 0;

    static std::shared_ptr<server> create();
};

typedef std::shared_ptr<server> server_ptr;

} // namespace stratum

#endif