#include <iostream>
#include <rpc_server.hpp>

using namespace std;

void test();

int main(int argc, const char *argv[])
{
    test();
}

void test()
{
    //std::tuple t{42, 10, 4.2}; // Another C++17 feature: class template argument deduction
    //std::apply([](auto&... args) {(( args =1 ), ...);}, t);
    //std::apply([](auto&&... args) {((std::cout << args << '\n'), ...);}, t);

    auto add = [](int a, int b, int c) -> int { 
        cout << __func__ << " : "<< a + b  + c << endl;
        return a + b + c; };
    auto sub = [](int a, int b) {
        cout << __func__ << " : " << a - b << endl;
        return a - b;
    };
    auto hello = [](string s) {
        cout << __func__ << " : " << s << endl;
        return s;
    };

    auto  server = rpc::server::create();

    server->add_handler("add", add);
    server->add_handler("sub", sub);
    server->add_handler("hello", hello);
    
    // server.call("add", 1, 2, 3);
    // server.call("sub", 10, 6);
    // server.call("hello", "Hello C++ 17");
    rpc::client client;
    
    auto obj = client.call("add", 1, 2, 3);
    
    server->exec(obj.first, obj.second);
}