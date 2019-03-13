#include "rpc_client.hpp"
#include <boost/asio.hpp>
#include <memory>

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::system::error_code;

namespace rpc
{
typedef deque<string> message_deque;

class client_imp : public client
{
public:
  virtual ~client_imp() {}

  client_imp() : //boost::asio::io_service &io_service
                 //m_io_service(io_service),
                 m_socket(m_io_service)
  {
  }

  virtual void connect(const std::string &ip, unsigned short port)
  {
    tcp::resolver resolver(m_io_service);
    auto endpoint_iterator = resolver.resolve({ip, to_string(port)});

    //tcp::endpoint endpoint(endpoint_iterator, port);
  }

  virtual void async_connect(const std::string &ip, unsigned short port, connect_handler handler)
  {
    tcp::resolver resolver(m_io_service);
    auto endpoint_iterator = resolver.resolve({ip, to_string(port)});

    boost::asio::async_connect(m_socket, endpoint_iterator,
                               [&, this](boost::system::error_code ec, tcp::resolver::iterator) {
                                 if (!ec)
                                 {
                                   handler({ec.value(), ec.message()});
                                 }
                               });

    //std::thread t([&](){ m_io_service.run(); });
    //t.detach();
  }

  virtual void run()
  {
    m_io_service.run();
  }

protected:
  virtual void write(std::string request)
  {
    cout << request << endl;
  }

  virtual void async_write(std::string request, write_handler &&handler)
  {
    boost::asio::async_write(m_socket,
                             boost::asio::buffer(request.c_str(), request.length()),
                             [&, cb = move(handler)](boost::system::error_code ec, std::size_t /*length*/) {
                               cb({ec.value(), ec.message()});

                               if (ec)
                               {
                                 m_socket.close();
                               }
                             });
  }

  virtual std::string read()
  {
    return "";
  }

  virtual void async_read(read_handler &&handler)
  {
    async_read_until(m_socket, m_streambuf, "\n",
                     [&, cb = move(handler)](const boost::system::error_code &ec, std::size_t /*length*/) {
                       if (!ec)
                       {
                         std::istream is(&m_streambuf);
                         //string response(istreambuf_iterator<char>(m_streambuf), istreambuf_iterator<char>() );
                         string response = "";

                         cb( {ec.value(), ec.message()}, response);
                       }
                       else
                       {
                         m_socket.close();
                       }
                     });
  }

private:
  // void do_connect(tcp::resolver::iterator endpoint_iterator)
  // {
  //   boost::asio::async_connect(m_socket, endpoint_iterator,
  //                              [this](boost::system::error_code ec, tcp::resolver::iterator) {
  //                                if (!ec)
  //                                {
  //                                  do_read_header();
  //                                }
  //                              });
  // }

  // void do_read_header()
  // {
  //   boost::asio::async_read(m_socket,
  //                           boost::asio::buffer(read_msg_.data(), chat_message::header_length),
  //                           [this](boost::system::error_code ec, std::size_t /*length*/) {
  //                             if (!ec && read_msg_.decode_header())
  //                             {
  //                               do_read_body();
  //                             }
  //                             else
  //                             {
  //                               m_socket.close();
  //                             }
  //                           });
  // }

  // void do_read_body()
  // {
  //   boost::asio::async_read(socket_,
  //                           boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
  //                           [this](boost::system::error_code ec, std::size_t /*length*/) {
  //                             if (!ec)
  //                             {
  //                               std::cout.write(read_msg_.body(), read_msg_.body_length());
  //                               std::cout << "\n";
  //                               do_read_header();
  //                             }
  //                             else
  //                             {
  //                               socket_.close();
  //                             }
  //                           });
  // }

  void do_write()
  {
    boost::asio::async_write(m_socket,
                             boost::asio::buffer(m_write_msgs.front().data(),
                                                 m_write_msgs.front().length()),
                             [this](boost::system::error_code ec, std::size_t /*length*/) {
                               if (!ec)
                               {
                                 m_write_msgs.pop_front();
                                 if (!m_write_msgs.empty())
                                 {
                                   do_write();
                                 }
                               }
                               else
                               {
                                 m_socket.close();
                               }
                             });
  }

private:
  boost::asio::io_service m_io_service;
  tcp::socket m_socket;
  boost::asio::streambuf m_streambuf;
  message_deque m_write_msgs;
};

std::shared_ptr<client> client::create()
{
  return make_shared<client_imp>();
}

} // namespace rpc