#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "banker/networker/socket.hpp"

using namespace banker;

void server()
{
    networker::socket s;
    s.set_reuse_address();
    if (!s.create())
    {
        std::cerr << "Failed to create server socket\n";
        return;
    }

    if (!s.bind(8080))
    {
        std::cerr << "Failed to bind\n";
        return;
    }

    if (!s.listen(1))
    {
        std::cerr << "Failed to listen\n";
        return;
    }

    std::cout << "Server listening\n";

    networker::socket client = s.accept();
    if (!client.is_initialized())
    {
        std::cerr << "Accept failed\n";
        return;
    }

    char buf[128];
    if (int n = client.recv(buf, sizeof(buf)); n > 0)
    {
        std::cout << "Server received: " << std::string(buf, n) << "\n";
        client.send("Hello from server", 17);
    }
}

void client()
{
    networker::socket s;
    if (!s.create())
    {
        std::cerr << "Failed to create client socket\n";
        return;
    }

    if (!s.connect("127.0.0.1", 8080))
    {
        std::cerr << "Failed to connect\n";
        return;
    }

    s.send("Hello from client", 17);
    char buf[128];
    int n = s.recv(buf, sizeof(buf));
    if (n > 0)
        std::cout << "Client received: " << std::string(buf, n) << "\n";
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
