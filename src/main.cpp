#include <fstream>
#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "../include/banker/networker/core/socket.hpp"
#include "../include/banker/networker/client.hpp"
#include "banker/crypto/format_bytes.hpp"
#include "banker/networker/server.hpp"

#include "banker/monocypher/monocypher.hpp"
#include "banker/monocypher/monocypher-ed25519.hpp"

#include "banker/common/rng/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"

#include "banker/networker/core/packet.hpp"

using namespace banker;

void test_handshake()
{
    uint8_t alice_sk[32];
    uint8_t alice_pk[32];
    uint8_t bob_sk[32];
    uint8_t bob_pk[32];

    uint8_t shared_a[32];
    uint8_t shared_b[32];

    uint8_t final_key_a[32];
    uint8_t final_key_b[32];

    for (int i = 0; i < 32; i++)
    {
        alice_sk[i] = rand() & 0xFF;
        bob_sk[i]   = rand() & 0xFF;
    }

    crypto_x25519_public_key(alice_pk, alice_sk);
    crypto_x25519_public_key(bob_pk, bob_sk);

    crypto_x25519(shared_a, alice_sk, bob_pk);
    crypto_x25519(shared_b, bob_sk, alice_pk);

    crypto_blake2b(final_key_a, sizeof(final_key_a), shared_a, sizeof(shared_a));
    crypto_blake2b(final_key_b, sizeof(final_key_b), shared_b, sizeof(shared_b));

    printf("Alice key: ");
    for (int i = 0; i < 32; i++) printf("%02x", final_key_a[i]);
    printf("\n");

    printf("Bob key:   ");
    for (int i = 0; i < 32; i++) printf("%02x", final_key_b[i]);
    printf("\n");

    if (memcmp(final_key_a, final_key_b, 32) == 0)
        printf("Handshake SUCCESS! Keys match.\n");
    else
        printf("Handshake FAILED! Keys differ.\n");
}

struct packet_test_1
{
    int x;
    int y;
};

void server()
{
    networker::packet p;
    const std::vector<std::vector<std::string>> data = {{"a","b"},{"c", "d"}};
    const std::vector<packet_test_1> data2 = {{0,0},{1,1},{100000000,1000000}};
    p.write(data);
    p.write(data2);

    auto v = p.read<std::vector<std::vector<std::string>>>();
    for (auto i : v)
    {
        for (auto ii : i)
        {
            std::cout << ii << ' ';
        }
        std::cout << std::endl;
    }

    auto v2 = p.read<std::vector<packet_test_1>>();
    for (auto packet : v2)
    {
        std::cout << "{" << packet.x << ", " << packet.y << "}" << std::endl;
    }

    auto v3_underflow = p.read<char>();
}

void client()
{
    test_handshake();
    networker::client client = {"127.0.0.1", 8080};
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
