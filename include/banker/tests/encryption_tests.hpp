//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_ENCRYPTION_TESTS_HPP
#define BANKER_ENCRYPTION_TESTS_HPP

#include "banker/core/crypto/crypter.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "banker/core/networker/crypto/crypto_core.hpp"
#include "banker/tester/tester.hpp"

BANKER_TEST_CASE(packet_encryption_and_decryption, simple, "Tries to encypt and decrypt a very simple packet.")
{
    std::vector<uint8_t> stream{};
    banker::crypter::key s_key;
    banker::crypto_rng::get(s_key);

    BANKER_MSG("key: ", banker::format_bytes::to_hex(s_key));
    BANKER_MSG("");
    std::vector<uint8_t> binary_plain_text;

    {
        banker::networker::crypto_core cc{};

        banker::networker::packet pkt{};
        banker::networker::packet data{};
        std::string s = "Hello, World!";
        data.write(s);
        BANKER_MSG("plain data: "," (",
                banker::format_bytes::to_hex(data.get_data())
            ,") = ", s);

        binary_plain_text = std::vector<uint8_t>{
            data.get_data().begin(),
            data.get_data().end()};

        auto mac = banker::networker::crypto_core::encrypt_packet(
            data,
            s_key,
            cc.generate_outgoing_nonce());



        BANKER_MSG("encrypted data: ", "(",
                banker::format_bytes::to_hex(data.get_data())
            ,")");
        BANKER_MSG("mac: ", banker::format_bytes::to_hex(mac));
        pkt.write(mac);
        pkt.write(data);
        pkt.serialize_into_stream(stream);
    }
    BANKER_MSG("");
    {
        banker::networker::crypto_core cc{};
        banker::networker::packet pkt = banker::networker::packet::deserialize(stream);
        bool valid = false;

        const auto mac = pkt.read<banker::crypter::mac>(&valid);
        if (valid == false) BANKER_FAIL("can't read the mac.");
        BANKER_MSG("mac: ", banker::format_bytes::to_hex(mac));

        auto data = pkt.read<banker::networker::packet>(&valid);
        if (valid == false) BANKER_FAIL("can't read the data.");
        BANKER_MSG("encrypted data: ", "(",
                banker::format_bytes::to_hex(data.get_data())
            ,")");

        banker::networker::crypto_core::decrypt_packet(
            data,
            s_key,
            cc.generate_incoming_nonce(),
            mac);

        std::string s = data.read<std::string>();
        BANKER_MSG("decrypted data: "," (",
                banker::format_bytes::to_hex(data.get_data())
            ,") = ", s);

        if (std::memcmp(
            data.get_data().data(),
            binary_plain_text.data(),
            data.get_data().size()
        ) != 0)
        {
            BANKER_FAIL("decryption failed.");
        }
    }
}

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
        BANKER_FAIL("Cannot be decrypted, invalid KEY, MAC or NONCE got modified.");
    }
    if (memcmp(plain.data(), str.data(), plain.size()) != 0)
    {
        BANKER_FAIL("Got decrypted but got wrong plaintext.");
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

#endif //BANKER_ENCRYPTION_TESTS_HPP