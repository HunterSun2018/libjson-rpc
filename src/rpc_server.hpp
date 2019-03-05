#ifndef STRATUM_SERVER_H
#define STRATUM_SERVER_H

#include "std.hpp"
#include "rpc_base.hpp"

namespace rpc
{

struct server : public server_base
{
    virtual ~server(){};

    //server(const server& s) = delete;
    //server& operator=(const server& s) = delete;

    //server(const server&& s);
    //server& operator=(const server&& s);

    virtual bool listen(unsigned short port) = 0;

    virtual bool run() = 0;

    virtual void stop() = 0;

    static std::shared_ptr<server> create(unsigned short port);
};

typedef std::shared_ptr<server> server_ptr;

} // namespace rpc

#endif