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
#include "banker/core/networker/crypto/crypto_channel_core.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"

using namespace banker;


[[noreturn]] void server()
{
    networker::packet_stream_host h{};
    bool w = h.create(5050,"127.0.0.1");
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

    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(100));

    while (true)
    {
        ss.tick();
        networker::packet p = ss.receive_packet();
        if (p.is_valid())
        {
            std::cout << "packet encrypted: " << format_bytes::to_hex(p.get_data()) << std::endl;
            networker::crypto_core::decrypt_packet(p,{1},{2});
            std::cout << "packet plaintext: " << format_bytes::to_hex(p.get_data()) << std::endl;

            std::cout << p.read<std::string>() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void client()
{
    networker::packet_stream_core core{};
    networker::socket s{};
    s.create();
    bool con = s.connect("127.0.0.1",5050);
    if (!con) std::cerr << "cant connect to server!" << std::endl;

    std::string msg = "Hello World!";
    networker::packet p{};
    p.write(msg);

    networker::crypto_core::encrypt_send(
        s,
        p,
        {1},
        core,
        {2});

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
