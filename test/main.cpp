#include <iostream>
#include "rpc_server.hpp"
#include "rpc_client.hpp"

using namespace std;

void test();

int main(int argc, const char *argv[])
{
    try
    {
        test();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void test()
{
    //std::tuple t{42, 10, 4.2}; // Another C++17 feature: class template argument deduction
    //std::apply([](auto&... args) {(( args =1 ), ...);}, t);
    //std::apply([](auto&&... args) {((std::cout << args << '\n'), ...);}, t);

    auto add = [](int a, int b, int c) -> int { 
        cout << __func__ << " : "<< a + b  + c << endl;
        return a + b + c; };
    auto sub = [](const string &a, int b) {
        cout << __func__ << " : " << a << b << endl;
        return b;
    };
    auto hello = [](string s) {
        cout << __func__ << " : " << s << endl;
        return string("Hello") + s;
    };

    auto server = rpc::server::create(8500);

    server->add_handler("add", add);
    server->add_handler("sub", sub);
    server->add_handler("hello", hello);

    std::thread job1([&]() { server->run(); });

    // server.call("add", 1, 2, 3);
    // server.call("sub", 10, 6);
    // server.call("hello", "Hello C++ 17");
    auto client = rpc::client::create("localhost", 8500);

    //client->connect("localhost", 8500);

    auto ret = client->call(0, "add", 1, 2, "3", string("4"), 5.0f);

    cout << ret << endl;

    auto ret1 = client->call(0, "sub", make_pair("name", "tom"), make_pair("age", 10));

    cout << ret1 << endl;

    client->async_call(0, "add", make_tuple(1, 2, 3), [&](rpc::client::status s, int ret) {
        if (!s.error)
        {
            cout << "call methoed 'add' and then get result : " << ret << endl;
        }
    });

    client->async_call(1, "sub", make_tuple(10, 5), [&](rpc::client::status s, int ret) {
        if (!s.error)
        {
            cout << "call methoed 'sub' and then get result : " << ret << endl;
        }
    });

    client->async_call(2, "hello", make_tuple("lib_json_rpc"), [&](rpc::client::status s, string ret) {
        if (!s.error)
        {
            cout << "call methoed 'hello' and then get result : " << ret << endl;
        }
    });

    client->run();
    cout << "client run ending" << endl;

    //server->exec(obj.first, obj.second);

    job1.join();
}