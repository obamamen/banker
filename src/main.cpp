#include <fstream>
#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "banker/common/formatting/time.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/client.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "banker/core/networker/server.hpp"


#include "banker/core/crypto/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"
#include "banker/core/crypto/crypter.hpp"

#include "banker/core/networker/core/packet.hpp"
#include "banker/core/networker/core/server/client_manager.hpp"
#include "banker/core/networker/core/tcp/packet_stream.hpp"
#include "banker/core/networker/crypto/crypto_channel_core.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"
#include "banker/tests/client_manager_tests.hpp"
#include "banker/tests/robin_hash_tests.hpp"

using namespace banker;


void server()
{
    networker::server server(5050);
    server.set_on_receive([&server](networker::server::client c, networker::packet packet)
    {
        std::string s = packet.read<std::string>();
        std::cout << c << ": " << s << std::endl;

        networker::packet op{};
        op.write(std::string("hello client"));

        server.send_packet(c,op);
    });

    // s.set_on_connect([&s](networker::server::client c)
    // {
    //     networker::packet op{};
    //     op.write(std::string("hello client"));
    //
    //     s.send_packet(c,op);
    // });

    while (true)
    {
        server.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void client()
{
    tester::run_test({"robin_hash"},false);

    //
    // networker::client client{"127.0.0.1",5050};
    // client.set_on_receive([](networker::packet p)
    // {
    //     std::cout << "Received: " << p.read<std::string>() << std::endl;
    // });
    // client.initiate_handshake();
    //
    // bool send = false;
    //
    // while (true)
    // {
    //     if (!send && client.allowed_to_send())
    //     {
    //         networker::packet op{};
    //         op.write(std::string("hello server"));
    //         client.send_packet(op);
    //         send = true;
    //     }
    //     client.tick();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }
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
