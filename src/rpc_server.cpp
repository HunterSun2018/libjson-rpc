//
//  Author : Hunter Sun
//
#include "std.hpp"
#include "jsonrp.hpp"
#include "rpc_server.hpp"
#include "utils.hpp"
#include <memory>

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
    session(tcp::socket &&socket, server_ptr server) : m_socket(move(socket)),
                                                       m_server(server)
    {
    }

    ~session()
    {
        BOOST_LOG_TRIVIAL(debug) << DEBUGGING_STRING << " session disconnected !";
    }

    void start()
    {
        //m_mine_pool.join(shared_from_this());

        read();
    }

protected:
    void read()
    {
        auto self(shared_from_this());

        //  the follow variable "self" captured in lambda adds the reference count during async reading
        //  once the seesion disconnects it will reduce the reference count and results in session deconstruction
        async_read_until(
            m_socket,
            m_streambuf, '\0',
            [this, self](const boost::system::error_code &ec, size_t length) {
                if (!ec)
                {
                    std::istream is(&m_streambuf);
                    auto iter = istreambuf_iterator<char>(is);
                    string request('\0', length + 1);

                    for (size_t i = 0; i < length; i++)
                        request += *iter++;

                    //vector<char> buffer(length + 1);
                    //is.read(&buffer[0], length);
                    //request = &buffer[0];

                    parse_json(request);

                    read();
                }
                else
                {
                    //m_mine_pool.leave(shared_from_this());
                }
            });
    }

    void write()
    {
        auto self(shared_from_this());

        async_write(m_socket,
                    buffer(m_msgs.front().data(), m_msgs.front().length() + 1), //contains char '\0' for seprating message
                    [this, self](boost::system::error_code ec, size_t size) {
                        if (!ec)
                        {
                            assert(size - 1 == m_msgs.front().length());

                            m_msgs.pop_front();

                            if (!m_msgs.empty())
                                write();
                        }
                        else
                        {
                            //m_mine_pool.leave(shared_from_this());
                        }
                    });
    }

    void parse_json(const string &json_obj)
    {
        string ret;

        try
        {
            jsonrpcpp::Parser parse;
            jsonrpcpp::entity_ptr entity = parse.parse(json_obj);
            if (entity && entity->is_request())
            {
                jsonrpcpp::request_ptr request = dynamic_pointer_cast<jsonrpcpp::Request>(entity);
                ostringstream oss;

                if (request->params.is_array())
                {
                    for (auto obj : request->params.param_array)
                        oss << obj.dump() << " ";
                }
                else if (request->params.is_map())
                {
                    for (auto iter : request->params.param_map)
                        oss << iter.second.dump() << " ";
                }

                auto result = m_server->exec(request->method, oss.str());

                jsonrpcpp::Response response(*request, result);

                ret = response.to_json().dump();
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            ret = e.what();
        }

        write_response(ret);
    }

    void write_response(const string &response)
    {
        cout << " Response: " << response << "\n";
        bool write_in_progress = !m_msgs.empty();

        m_msgs.push_back(response); //append "\n" as a newline

        if (!write_in_progress)
        {
            write();
        }
    }

private:
    tcp::socket m_socket;
    boost::asio::streambuf m_streambuf;
    message_deque m_msgs;
    server_ptr m_server;
};

class server_imp : public server,
                   public std::enable_shared_from_this<server_imp>
{
public:
    server_imp(unsigned short port = default_listen_port)
        : m_io_service(utils::g_io_service::instance()),
          m_socket(m_io_service),
          m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port)), //default, listen port 8500          
          m_timer(m_io_service, boost::posix_time::seconds(timer_interval))
    {
    }

    virtual ~server_imp()
    {
    }

    virtual bool listen(unsigned short port) //user:pwd
    {
        tcp::endpoint endpoint(tcp::v4(), port);

        m_acceptor.close();

        m_acceptor = tcp::acceptor(m_io_service, endpoint);

        BOOST_LOG_TRIVIAL(info) << DEBUGGING_STRING
                                << "\n  Server init, port = " << port
                                << endl;

        return true;
    }

    virtual bool run()
    {
        accept();

        //start a timer to periodically call get block template
        timer_event_getblocktemplate();

        BOOST_LOG_TRIVIAL(info) << DEBUGGING_STRING << "Server run" << endl;

        //m_io_service.run();

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
                make_shared<session>(move(m_socket), shared_from_this())->start();
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
    boost::asio::io_service &m_io_service;
    tcp::socket m_socket;
    tcp::acceptor m_acceptor;
    enum
    {
        default_listen_port = 8500,
        timer_interval = 15, //timer inverval 15 seconds for getblcoktemplate
    };

    deadline_timer m_timer;
};

server_ptr rpc::server::create(unsigned short port)
{
    return make_shared<rpc::server_imp>(port);
}

} // namespace rpc
