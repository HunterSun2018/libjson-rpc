//
//  Author : Hunter Sun
//
#include "std.hpp"
#include "jsonrp.hpp"
#include "rpc_server.hpp"
#include "utils.h"
//
// header files for boost
//
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;

typedef boost::system::error_code bsec;

namespace rpc
{

//------------------------------------------------------------
typedef deque<string> message_deque;

//
//  session for tcp connection
//
class session : public std::enable_shared_from_this<session>
{
  public:
    session(tcp::socket &&socket) : m_socket(move(socket))
    {
    }

    ~session()
    {
        BOOST_LOG_TRIVIAL(debug) << DEBUGGING_STRING << " session disconnected !";
    }

    void start()
    {
        //m_mine_pool.join(shared_from_this());

        read_json();
    }

    //
    //  deliver mining message
    //
    virtual void deliver(const string &json_rpc)
    {
        bool write_in_progress = !m_msgs.empty();

        m_msgs.push_back(json_rpc + "\n"); //append "\n" as a newline

        if (!write_in_progress)
        {
            write_json();
        }
    }

  protected:
    void read_json()
    {
        auto self(shared_from_this());

        //  the follow variable "self" captured in lambda adds the reference count during async reading
        //  once the seesion disconnects it will reduce the reference count and results in session deconstruction
        async_read_until(m_socket, m_streambuf, "\n",
                         [this, self](const boost::system::error_code &ec, const long unsigned int &a) {
                             if (!ec)
                             {
                                 std::istream input(&m_streambuf);
                                 string line;
                                 getline(input, line);

                                 //m_mine_pool.rpc_call(line, shared_from_this());

                                 read_json();
                             }
                             else
                             {
                                 //m_mine_pool.leave(shared_from_this());
                             }
                         });
    }

    void write_json()
    {
        auto self(shared_from_this());

        async_write(m_socket,
                    buffer(m_msgs.front().data(), m_msgs.front().length()),
                    [this, self](boost::system::error_code ec, size_t size) {
                        if (!ec)
                        {
                            assert(size == m_msgs.front().length());

                            m_msgs.pop_front();

                            if (!m_msgs.empty())
                                write_json();
                        }
                        else
                        {
                            //m_mine_pool.leave(shared_from_this());
                        }
                    });
    }

  private:
    tcp::socket m_socket;
    boost::asio::streambuf m_streambuf;
    message_deque m_msgs;
};

class server_imp : public server
{
  public:
    server_imp()
        : m_socket(m_io_service),
          m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), default_listen_port)), //default, listen port 8500
          //m_mine_pool(bind(&create_http_client, &m_io_service), "", ""),           //defalut data dir
          m_timer(m_io_service, boost::posix_time::seconds(timer_interval))
    {
    }

    virtual ~server_imp()
    {
    }

    virtual bool init(unsigned short port,                 //server listening port
                      const std::string &network_node_url, //node rpc url
                      const std::string &user_pwd)         //user:pwd
    {
        tcp::endpoint endpoint(tcp::v4(), port);

        m_acceptor.close();

        m_acceptor = tcp::acceptor(m_io_service, endpoint);

        BOOST_LOG_TRIVIAL(info) << DEBUGGING_STRING
                                << "\n  Server init, port = " << port
                                << "\n  Network node url = " << network_node_url
                                << "\n  User:pwd = " << user_pwd
                                << endl;

        return true;
    }

    virtual bool run()
    {
        accept();

        //start a timer to periodically call get block template
        timer_event_getblocktemplate();

        BOOST_LOG_TRIVIAL(info) << DEBUGGING_STRING << "Server run" << endl;

        m_io_service.run();

        return true;
    }

    virtual void stop()
    {
        return;
    }

  protected:
    void accept()
    {
        m_acceptor.async_accept(m_socket, [this](boost::system::error_code ec) {
            if (!ec)
            {
                make_shared<session>(move(m_socket))->start();
            }

            accept();

            BOOST_LOG_TRIVIAL(debug) << DEBUGGING_STRING << "\n\t"
                                     << "accept a new connection, status is " << ec.message()
                                     << endl;
        });
    }

    void timer_event_getblocktemplate()
    {
        m_timer.expires_at(m_timer.expires_at() + boost::posix_time::seconds(timer_interval));
        m_timer.async_wait(bind(&server_imp::timer_event_getblocktemplate, this));
    }

  private:
    boost::asio::io_service m_io_service;
    tcp::socket m_socket;
    tcp::acceptor m_acceptor;
    enum
    {
        default_listen_port = 8500,
        timer_interval = 15, //timer inverval 15 seconds for getblcoktemplate
    };

    deadline_timer m_timer;
};

server_ptr rpc::server::create()
{
    return make_shared<rpc::server_imp>();
}

} // namespace rpc
