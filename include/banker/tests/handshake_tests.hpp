//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_HANDSHAKE_TESTS_HPP
#define BANKER_HANDSHAKE_TESTS_HPP

#include "banker/core/crypto/crypter.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "banker/tester/tester.hpp"

BANKER_TEST_CASE(handshake, base, "Tests a simple handshake between a and b")
{
    banker::crypter::handshake a = banker::crypter::handshake();
    banker::crypter::handshake b = banker::crypter::handshake();

    BANKER_MSG("a::public_key = ", banker::format_bytes::to_hex(a.get_public()));
    BANKER_MSG("b::public_key = ", banker::format_bytes::to_hex(b.get_public()));

    BANKER_MSG("a::private_key = ", banker::format_bytes::to_hex(a.get_private()));
    BANKER_MSG("b::private_key = ", banker::format_bytes::to_hex(b.get_private()));


    a.generate_shared_secret(b.get_public());
    b.generate_shared_secret(a.get_public());


    BANKER_MSG("");
    BANKER_MSG("a::shared_secret = ", banker::format_bytes::to_hex(a.get_shared_secret()));
    BANKER_MSG("b::shared_secret = ", banker::format_bytes::to_hex(b.get_shared_secret()));
    if (a.get_shared_secret() != b.get_shared_secret())
    {
        BANKER_FAIL("a's and b's shared secret are not the same the same.");
    }
}

BANKER_TEST_CASE(handshake, modified, "Tries to sabotage a's public key to stop the handshake with b")
{
    banker::crypter::handshake a = banker::crypter::handshake();
    banker::crypter::handshake b = banker::crypter::handshake();

    banker::crypter::key ap = a.get_public();
    banker::crypto_rng::get_bytes(ap.bytes, sizeof(ap.bytes));
    BANKER_MSG("a::public_key = ", banker::format_bytes::to_hex(ap));
    BANKER_MSG("b::public_key = ", banker::format_bytes::to_hex(b.get_public()));

    BANKER_MSG("a::private_key = ", banker::format_bytes::to_hex(a.get_private()));
    BANKER_MSG("b::private_key = ", banker::format_bytes::to_hex(b.get_private()));


    a.generate_shared_secret(b.get_public());
    b.generate_shared_secret(ap);


    BANKER_MSG("");
    BANKER_MSG("a::shared_secret = ", banker::format_bytes::to_hex(a.get_shared_secret()));
    BANKER_MSG("b::shared_secret = ", banker::format_bytes::to_hex(b.get_shared_secret()));
    if (a.get_shared_secret() == b.get_shared_secret())
    {
        BANKER_FAIL("a's and b's shared secret are not the same the same.");
    }
}

#endif //BANKER_HANDSHAKE_TESTS_HPP