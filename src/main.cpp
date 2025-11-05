#include <fstream>
#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "../include/banker/networker/core/socket.hpp"
#include "../include/banker/networker/client.hpp"
#include "banker/crypto/format_bytes.hpp"
#include "banker/networker/server.hpp"


#include "../include/banker/crypto/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"
#include "banker/crypto/crypter.hpp"

#include "banker/networker/core/packet.hpp"

using namespace banker;

struct packet_test_1
{
    int x;
    int y;
};

void server()
{
    auto server = networker::server(5050);

    server.set_on_connect(
        [](const networker::server::client_id id)
        {
            std::cout << "Client " << id << " connected\n";
        }
    );

    server.set_on_disconnect(
    [](const networker::server::client_id id)
        {
            std::cout << "Client " << id << " disconnected\n";
        }
    );

    server.set_on_receive([](const networker::server::client_id id, networker::packet packet)
        {
            std::cout << "Client " << id << " send: " << packet.read<std::string>() << std::endl;
        }
    );

    server.set_on_error(
    [](const networker::server::client_id id,std::string msg)
        {
            std::cout << "Client " << id << " error: " << msg << std::endl;
        }
    );

    server.run();
}

void client()
{
    networker::client client = {"127.0.0.1", 5050};
    networker::client client2 = {"127.0.0.1", 5050};
    networker::client client3 = {"127.0.0.1", 5050};

    while (true)
    {
        client.tick();
        client2.tick();
        client3.tick();
    }
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
