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
#include "banker/core/networker/core/tcp/packet_stream.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"

using namespace banker;



void server()
{
    networker::stream_socket_host h{};
    bool w = h.create_and_bind(5050,"127.0.0.1");
    if (w) std::cout << "Server listening on port 5050" << std::endl;
    networker::packet_stream ss;
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
    networker::packet_stream c{};
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
