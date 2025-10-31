#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "banker/networker/socket.hpp"

int main()
{
    banker::networker::socket s;
    std::thread t;
    std::cout << "Hello, World!" << std::endl;
}
