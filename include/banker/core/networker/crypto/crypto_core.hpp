/* ================================== *\
 @file     crypto_core.hpp
 @project  banker
 @author   moosm
 @date     11/17/2025
*\ ================================== */

#ifndef BANKER_CRYPTO_CORE_HPP
#define BANKER_CRYPTO_CORE_HPP
#include <cstdint>
#include <cstring>

#include "banker/core/crypto/crypter.hpp"
#include "banker/core/networker/core/packet.hpp"

namespace banker::networker
{
    class crypto_core
    {
    public:
        crypto_core()                               = default;
        ~crypto_core()                              = default;

        crypto_core(const crypto_core&)             = delete;
        crypto_core& operator=(const crypto_core&)  = delete;

        crypto_core(crypto_core&&) = default;
        crypto_core& operator=(crypto_core&&)       = default;

        BANKER_NODISCARD static crypter::mac encrypt_packet(
            packet& packet,
            const crypter::key& shared_key,
            const crypter::nonce& nonce,
            const std::span<uint8_t> extra_data = {})
        {
            crypter::mac result;
            crypter::encrypt(
                shared_key,
                packet.get_remaining_data(),
                extra_data,
                nonce,
                result);

            return result;
        }

        bool static decrypt_packet(
            packet& packet,
            const crypter::key& shared_key,
            const crypter::nonce& nonce,
            const crypter::mac& hmac,
            const std::span<uint8_t> extra_data = {})
        {
            return crypter::decrypt(
                shared_key,
                packet.get_remaining_data(),
                extra_data,
                nonce,
                hmac);
        }

        crypter::nonce generate_incoming_nonce() const
        {
            crypter::nonce n{};

            std::memcpy(
                n.bytes,
                &_in_counter,
                sizeof(_in_counter));

            return n;
        }

        crypter::nonce generate_outgoing_nonce() const
        {
            crypter::nonce n{};

            std::memcpy(
                n.bytes,
                &_out_counter,
                sizeof(_out_counter));

            return n;
        }

        void increment_incoming(const size_t c = 1) { _in_counter+=c; }

        void increment_outgoing(const size_t c = 1) { _out_counter+=c; }

    private:
        uint64_t _out_counter{0};
        uint64_t _in_counter{0};

    };
}

#endif //BANKER_CRYPTO_CORE_HPP