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

void server()
{
    auto server = networker::server(5050);

    server.set_on_connect(
        [](const networker::server::client_id id)
        {
            std::cout << banker::common::formatting::get_current_time(true) << "(+) client " << id << " connected\n";
        }
    );

    server.set_on_disconnect(
    [](const networker::server::client_id id)
        {
            std::cout << banker::common::formatting::get_current_time(true) << "(-) client " << id << " disconnected\n";
        }
    );

    server.set_on_receive([](const networker::server::client_id id, networker::packet packet)
        {
            std::cout << banker::common::formatting::get_current_time(true) << "[*] client " << id << " send: " << packet.read<std::string>() << std::endl;
        }
    );

    server.set_on_internal_message([](const std::string& msg)
        {
            std::cout << banker::common::formatting::get_current_time(true) << "<server> " << msg << std::endl;
        }
    );


    server.run();
}

void client()
{
    networker::client client = {"127.0.0.1", 5050};

    while (true)
    {
        client.tick();
        std::cout << banker::common::formatting::get_current_time(true) << "tick()" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
