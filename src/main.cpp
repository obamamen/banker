#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "../include/banker/networker/core/socket.hpp"
#include "../include/banker/networker/client.hpp"
#include "banker/networker/server.hpp"

using namespace banker;

void server()
{
    networker::server server(8080);
    while (true)
    {
        server.accept_backlog();
        server.log_resv();
    }
}

void client()
{
    networker::client client = {"127.0.0.1", 8080};
}

int main()
{
#ifdef BUILD_CLIENT
client();
#elif defined(BUILD_SERVER)
server();
#else
    std::cout << "Unknown mode\n";
#endif
}
