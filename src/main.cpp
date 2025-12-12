#include <fstream>
#include <iostream>
#include <list>
#include <thread>
#include <filesystem>
#include <sstream>
#include <iomanip>

#include "banker/banker.hpp"
#include "banker/common/formatting/time.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/crypto/format_bytes.hpp"

#include "banker/core/crypto/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"
#include "banker/core/crypto/crypter.hpp"

#include "banker/core/networker/core/packet/packet.hpp"
#include "banker/core/networker/core/tcp/stream_core.hpp"
#include "../old/crypto/crypto_channel_core.hpp"
#include "banker/core/networker/core/stream_socket/stream_socket.hpp"
#include "banker/core/networker/core/stream_socket/stream_socket_core.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"
#include "banker/tests/robin_hash_tests.hpp"

#include "http_server.hpp"

using namespace banker;
namespace fs = std::filesystem;

void log_char_vector(const std::vector<uint8_t>& vec)
{
    for (const auto& i : vec) std::cout << static_cast<char>(i);
}

void server()
{
    networker::stream_socket::acceptor server("0.0.0.0",8080);
    std::list<networker::stream_socket> clients;
    while (true)
    {
        while (server.touch())
        {
            networker::stream_socket new_client = server.accept();
            if (new_client.is_valid())
                clients.push_back(std::move(new_client));
        }
        for (auto it = clients.begin(); it != clients.end(); )
        {
            auto& client = *it;
            networker::stream_socket_core::request_result result;
            const auto r = client.tick(true, false, &result);
            if (result != networker::stream_socket_core::request_result::ok)
            {
                std::cout << "[server] client disconnected\n";
                it = clients.erase(it);
                continue;
            }
            if (r != 0)
            {
                std::cout << "[server] client("<<&it<<") : ";
                log_char_vector(client.receive());
                std::cout << "\n";
                client.receive().clear();
            }
        }
    }
}

[[noreturn]] void client()
{
    networker::stream_socket client("127.0.0.1",8080);
    std::string msg = "Hello Server!";
    client.enqueue({msg.begin(), msg.end()});
    while (true)
    {
        if (client.raw_socket().is_writable()) client.tick(false,true);
    }
}

int main()
{
#ifdef BUILD_CLIENT
    client();
#elif defined(BUILD_SERVER)
    server();
#elif defined(BUILD_TESTS)
    tester::run_test(true);
#elif defined(BUILD_HTTP_FILE_SERVER)
    http_server(false);
#else
    std::cerr << "Unknown mode\n";
    return 1;
#endif
}