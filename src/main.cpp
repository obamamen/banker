#include <fstream>
#include <iostream>
#include <thread>

#include "banker/banker.hpp"
#include "../include/banker/networker/core/socket.hpp"
#include "../include/banker/networker/client.hpp"
#include "banker/crypto/format_bytes.hpp"
#include "banker/networker/server.hpp"

#include "banker/monocypher/monocypher.h"
#include "banker/monocypher/monocypher-ed25519.h"

#include "banker/common/rng/crypto_rng.hpp"
#include "banker/common/time/timers.hpp"

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

void server()
{
    constexpr size_t rng2_l = 1024 * 1024 * 512;
    const auto rng2 = new uint8_t[rng2_l];
    {
        time::scoped_timer t("RNG2 GENERATOR",true);
        crypto_rng::get_bytes(rng2, rng2_l);
    }

    {
        time::scoped_timer t("RNG2 TO FILE",true);

        std::ofstream out("rng_output.txt", std::ios::out | std::ios::trunc);
        banker::format_bytes::to_hex_bytes_stream(rng2, rng2_l, out);
    }

    delete[] rng2;
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
