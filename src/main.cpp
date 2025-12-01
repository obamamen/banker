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
#include "banker/core/networker/core/tcp/stream_core.hpp"
#include "../old/crypto/crypto_channel_core.hpp"
#include "banker/tester/tester.hpp"

#include "banker/tests/encryption_tests.hpp"
#include "banker/tests/handshake_tests.hpp"
#include "banker/tests/packet_tests.hpp"
#include "banker/tests/client_manager_tests.hpp"
#include "banker/tests/robin_hash_tests.hpp"
#include "banker/tests/stream_network_tests.hpp"

using namespace banker;


void server()
{
}

void client()
{
}

int main()
{
#ifdef BUILD_CLIENT
    client();
#elif defined(BUILD_SERVER)
    server();
#elif defined(BUILD_TESTS)
    tester::run_test(false);
#else
    std::cerr << "Unknown mode\n";
    return 1;
#endif
}
