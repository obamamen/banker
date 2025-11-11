#include <fstream>
#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/client.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "banker/core/networker/server.hpp"


#include "banker/core/crypto/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"
#include "banker/core/crypto/crypter.hpp"

#include "banker/core/networker/core/packet.hpp"
#include "banker/core/networker/core/tcp/stream_socket.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"

using namespace banker;

// void server()
// {
//     auto server = networker::server(5050);
//
//     server.set_on_connect(
//         [](const networker::server::client_id id)
//         {
//             std::cout << banker::common::formatting::get_current_time(true) << "(+) client " << id << " connected\n";
//         }
//     );
//
//     server.set_on_disconnect(
//     [](const networker::server::client_id id)
//         {
//             std::cout << banker::common::formatting::get_current_time(true) << "(-) client " << id << " disconnected\n";
//         }
//     );
//
//     server.set_on_receive([](const networker::server::client_id id, networker::packet packet)
//         {
//             std::cout << banker::common::formatting::get_current_time(true) << "[*] client " << id << " send: " << packet.read<std::string>() << std::endl;
//         }
//     );
//
//     server.set_on_internal_message([](const std::string& msg)
//         {
//             std::cout << banker::common::formatting::get_current_time(true) << "<server> " << msg << std::endl;
//         }
//     );
//
//
//     server.run();
// }
//
// void client()
// {
//     banker::networker::client client("127.0.0.1",5050);
//     while (true)
//     {
//         client.tick();
//     }
//     //tester::run_test({}, true);
// }

[[noreturn]] void server()
{
    networker::stream_socket_host h{};
    bool w = h.create_and_bind(5050,"127.0.0.1");
    if (w) std::cout << "Server listening on port 5050" << std::endl;
    networker::stream_socket ss;
    while (true)
    {
        ss = h.accept();
        if (ss.is_valid())
        {
            std::cout << "client_connected" << std::endl;
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(1000));
    while (true)
    {
        ss.tick();
        networker::packet p = ss.receive_packet();
        if (p.is_valid())
        {
            std::cout << "server received: ";
            auto s = p.read<std::string>();
            std::cout << s[0] << "..." << " size: " << p.get_data().size() << std::endl;
        }
    }
}

void client()
{
    networker::stream_socket c{};
    bool con = c.connect(5050,"127.0.0.1");
    if (!con) std::cerr << "Failed to connect to server" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        networker::packet p{};
        std::string big_data(1024 * 1024, 'A' + (i % 26));
        p.write(big_data);
        auto s = c.send_packet(p);
        if (s.sent_current != 1)
            std::cout << "Packet " << i << " partially sent, queued in _send_buff\n";
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
