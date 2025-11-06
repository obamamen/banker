//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_ENCRYPTION_HPP
#define BANKER_ENCRYPTION_HPP

#include "banker/crypto/crypter.hpp"
#include "banker/crypto/format_bytes.hpp"
#include "tester/tester.hpp"

BANKER_TEST_CASE(encryption_and_decryption, hello_world, "Tries to encrypt and decrypt \"Hello world!\"")
{
    std::string str = "Hello world!";
    std::vector<uint8_t> plain(str.begin(), str.end());

    BANKER_MSG("Plain: ", banker::format_bytes::to_hex(plain));

    banker::crypter::key k{};
    banker::crypter::nonce n{};
    n.bytes[0] = 1;

    banker::crypto_rng::get_bytes(k.bytes, sizeof(k.bytes));

    banker::crypter::mac m{};
    banker::crypter::encrypt(k, plain, {}, n, m);
    BANKER_MSG("Encrypted: ", banker::format_bytes::to_hex(plain));

    bool valid = banker::crypter::decrypt(k, plain, {}, n, m);
    BANKER_MSG("Decrypted: ", banker::format_bytes::to_hex(plain));
    if (valid == false)
    {
        BANKER_FAIL("Cannot be decrypted");
    }
}

BANKER_TEST_CASE(encryption_and_decryption, hello_world_2, "Messes with the MAC while decrypting \"Hello world!\"")
{
    std::string str = "Hello world!";
    std::vector<uint8_t> plain(str.begin(), str.end());

    BANKER_MSG("Plain: ", banker::format_bytes::to_hex(plain));

    banker::crypter::key k{};
    banker::crypter::nonce n{};
    n.bytes[0] = 1;

    banker::crypto_rng::get_bytes(k.bytes, sizeof(k.bytes));

    banker::crypter::mac m{};
    banker::crypter::encrypt(k, plain, {}, n, m);
    BANKER_MSG("Encrypted: ", banker::format_bytes::to_hex(plain));
    m.bytes[0] = 89;
    m.bytes[1] = 123;
    m.bytes[2] = 210;
    bool valid = banker::crypter::decrypt(k, plain, {}, n, m);
    if (valid == true)
    {
        BANKER_FAIL("Got decrypted, should have been unable to because MAC has been modified");
    }
}

BANKER_TEST_CASE(encryption_and_decryption, hello_world_3, "Messes with the NONCE while decrypting \"Hello world!\"")
{
    std::string str = "Hello world!";
    std::vector<uint8_t> plain(str.begin(), str.end());

    BANKER_MSG("Plain: ", banker::format_bytes::to_hex(plain));

    banker::crypter::key k{};
    banker::crypter::nonce n{};
    n.bytes[0] = 1;

    banker::crypto_rng::get_bytes(k.bytes, sizeof(k.bytes));

    banker::crypter::mac m{};
    banker::crypter::encrypt(k, plain, {}, n, m);
    BANKER_MSG("Encrypted: ", banker::format_bytes::to_hex(plain));
    n.bytes[0] = 89;
    n.bytes[1] = 123;
    n.bytes[2] = 210;
    bool valid = banker::crypter::decrypt(k, plain, {}, n, m);
    BANKER_MSG("Decrypted: ", banker::format_bytes::to_hex(plain));
    if (valid == true)
    {
        BANKER_FAIL("Got decrypted, should have been unable to because NONCE has been modified");
    }
}

BANKER_TEST_CASE(encryption_and_decryption, hello_world_4, "Messes with the KEY while decrypting \"Hello world!\"")
{
    std::string str = "Hello world!";
    std::vector<uint8_t> plain(str.begin(), str.end());

    BANKER_MSG("Plain: ", banker::format_bytes::to_hex(plain));

    banker::crypter::key k{};
    banker::crypter::nonce n{};
    n.bytes[0] = 1;

    banker::crypto_rng::get_bytes(k.bytes, sizeof(k.bytes));

    banker::crypter::mac m{};
    banker::crypter::encrypt(k, plain, {}, n, m);
    BANKER_MSG("Encrypted: ", banker::format_bytes::to_hex(plain));
    k.bytes[0] = 89;
    k.bytes[1] = 123;
    k.bytes[2] = 210;
    bool valid = banker::crypter::decrypt(k, plain, {}, n, m);
    BANKER_MSG("Decrypted: ", banker::format_bytes::to_hex(plain));
    if (valid == true)
    {
        BANKER_FAIL("Got decrypted, should have been unable to because KEY has been modified");
    }
}

#endif //BANKER_ENCRYPTION_HPP