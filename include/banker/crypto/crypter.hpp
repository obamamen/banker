//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_CRYPTER_HPP
#define BANKER_CRYPTER_HPP

// VENDOR
#include "banker/vendor/monocypher/monocypher-ed25519.hpp"
#include "banker/vendor/monocypher/monocypher.hpp"

#include "banker/crypto/crypto_rng.hpp"


namespace banker::crypter
{
    struct key { uint8_t bytes[32]{}; };

    class handshake
    {
        bool _is_shared_valid = false;

        key _private{};
        key _public{};
        key _shared_secret{};

    public:
        handshake()
        {
            crypto_rng::get_bytes(_private.bytes, sizeof(_private));
            _generate_public();
        }

        [[nodiscard]] key get_public() const
        {
            return _public;
        }

        [[nodiscard]] key get_shared_secret() const
        {
            return _shared_secret;
        }

        void generate_shared_secret(const key &other_public)
        {
            key secret_unhashed{};
            crypto_x25519(secret_unhashed.bytes, _private.bytes, other_public.bytes);

            crypto_blake2b(
                _shared_secret.bytes, sizeof(_shared_secret),
                secret_unhashed.bytes, sizeof(secret_unhashed)
            );

            _is_shared_valid = true;
        }

        [[nodiscard]] bool is_shared_valid() const { return _is_shared_valid; }

    private:
        void _generate_public()
        {
            crypto_x25519_public_key( _public.bytes, _private.bytes );
        }
    };
}

#endif //BANKER_CRYPTER_HPP