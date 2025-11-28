#include <fstream>
#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "banker/common/formatting/time.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/crypto/format_bytes.hpp"


#include "banker/core/crypto/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"
#include "banker/core/crypto/crypter.hpp"

#include "banker/core/networker/core/packet/packet.hpp"
#include "banker/core/networker/core/server/client_manager.hpp"
#include "banker/core/networker/core/packet_streaming/packet_stream.hpp"
#include "banker/core/networker/core/tcp/stream_core.hpp"
#include "banker/core/networker/crypto/crypto_channel_core.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"
#include "banker/tests/client_manager_tests.hpp"
#include "banker/tests/robin_hash_tests.hpp"

using namespace banker;



#define BANKER_SHOULD(thing) do { if (!(thing)) throw std::runtime_error(#thing);  } while(0)

void server()
{
    networker::socket server_socket = networker::socket();
    BANKER_SHOULD(server_socket.create());
    BANKER_SHOULD(server_socket.set_reuse_address(true));
    BANKER_SHOULD(server_socket.set_blocking(false));
    BANKER_SHOULD(server_socket.bind(5050));
    BANKER_SHOULD(server_socket.listen());

    networker::socket client_socket = networker::socket();
    networker::stream_core stream_core_client_1 = networker::stream_core();


    std::string s = "Hello World!";

    while(true)
    {
        networker::socket a = server_socket.accept();
        if (a.is_valid())
        {
            std::cout << "Client connected" << std::endl;
            client_socket = std::move(a);
            stream_core_client_1.send_data(
                client_socket,
                reinterpret_cast<uint8_t*>( s.data() ),
                s.size()+1);

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        if (client_socket.is_valid())
        {
            stream_core_client_1.flush_send_buffer(client_socket);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

}

void print_uint8_array(const std::vector<uint8_t>& data)
{
    std::cout << "\"";
    for (const unsigned char i : data) std::cout << i;
    std::cout << "\"" << std::endl;
}

void client()
{
    networker::socket client_socket = networker::socket();
    BANKER_SHOULD(client_socket.create());
    BANKER_SHOULD(client_socket.connect("127.0.0.1",5050));
    BANKER_SHOULD(client_socket.set_blocking(false));

    networker::stream_core stream_core = networker::stream_core();

    while (true)
    {
        stream_core.tick_receive(client_socket);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (!stream_core.receive().empty())
            print_uint8_array(stream_core.receive());
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
