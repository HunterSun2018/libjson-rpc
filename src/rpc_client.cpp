#include "rpc_client.hpp"
#include <boost/asio.hpp>
#include <memory>
#include "utils.hpp"
#include "jsonrp.hpp"

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

  client_imp(std::string ip, uint16_t port) : m_io_service(utils::g_io_service::instance()),
                                              m_socket(m_io_service)
  {
    m_server_ip = ip;
    m_server_port = port;

    async_connect();
  }

  virtual std::string run(const std::string &request) override
  {
    cout << request << endl;
    return "";
  }

  virtual void async_remote_call(json_request &request, callback cb) override
  {
    cout << request.to_string() << endl;

    m_callback_map[request.id()] = cb;

    write(request.to_string());
  }

protected:
  void connect(const std::string &ip, unsigned short port)
  {
    tcp::resolver resolver(m_io_service);
    auto endpoint_iterator = resolver.resolve({ip, to_string(port)});

    //tcp::endpoint endpoint(endpoint_iterator, port);
  }

  void async_connect()
  {
    tcp::resolver resolver(m_io_service);
    auto endpoint_iterator = resolver.resolve({m_server_ip, to_string(m_server_port)});

    boost::asio::async_connect(
        m_socket,
        endpoint_iterator,
        [&, this](boost::system::error_code ec, tcp::resolver::iterator) {
          if (!ec)
          {
            read();
          }
          else
          {
            //status s{ec.value(), ec.message()};

            //cb(s, "");
            cout << ec.message() << endl;
            m_socket.close();
          }
        });
  }

  void parse_response(const string &json_obj)
  {
    try
    {
      jsonrpcpp::Parser parse;
      jsonrpcpp::entity_ptr entity = parse.parse(json_obj);

      if (entity && entity->is_response())
      {
        jsonrpcpp::response_ptr response = dynamic_pointer_cast<jsonrpcpp::Response>(entity);

        auto iter = m_callback_map.find(response->id.int_id);
        if (iter != end(m_callback_map))
        {
          status s = {0, ""};

          iter->second(s, response->result);
        }
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }

    //cout << json_obj << endl;
  }
  virtual void read()
  {
    boost::asio::async_read_until(
        m_socket,
        m_streambuf,
        '\0',
        [&](const boost::system::error_code &ec, std::size_t length) {
          if (!ec)
          {
            std::istream is(&m_streambuf);
            //string response = string(istreambuf_iterator<char>(is), istreambuf_iterator<char>());
            string response(length, 0);

            is.read(&response[0], length);

            parse_response(response);

            read();
          }
          else
          {
            m_socket.close();
          }
        });
  }

  void write(std::string request)
  {
    m_io_service.post(
        [this, request]() {
          bool write_in_progress = !m_write_msgs.empty();
          m_write_msgs.push_back(request);

          if (!write_in_progress)
          {
            do_write();
          }
        });
  }

  void do_write()
  {
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_write_msgs.front().data(),
                            m_write_msgs.front().length() + 1), // buffer contains the end of character '\0'
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
  boost::asio::io_service &m_io_service;
  tcp::socket m_socket;
  boost::asio::streambuf m_streambuf;
  message_deque m_write_msgs;
  string m_server_ip;
  ushort m_server_port;

  map<int, callback> m_callback_map;
};

// void client_imp::async_run(uint32_t id, const std::string &method, json_request &stream, std::function<void(status &s, std::string)> f)
// {
//   async_connect();
// }

std::shared_ptr<client> client::create(std::string ip, uint16_t port)
{
  return make_shared<client_imp>(ip, port);
}

} // namespace rpc