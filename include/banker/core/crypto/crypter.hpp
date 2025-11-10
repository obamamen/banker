//
// Created by moosm on 11/5/2025.
//

#ifndef BANKER_CRYPTER_HPP
#define BANKER_CRYPTER_HPP

// VENDOR
#include <span>

#include "banker/vendor/monocypher/monocypher-ed25519.hpp"
#include "banker/vendor/monocypher/monocypher.hpp"

#include "banker/core/crypto/crypto_rng.hpp"


namespace banker::crypter
{
    struct key { uint8_t bytes[32]{};

        bool operator==(const key& key) const = default;
        bool operator!=(const key& key) const = default;
    };
    struct nonce { uint8_t bytes[24]{}; };
    struct mac { uint8_t bytes[16]{}; };

    /// @brief encrypts anything, using KEY, and NONCE
    /// @param key shared key.
    /// @param data plaintext.
    /// @param extra_data plaintext , won't be changed will be calculated into the HMAC.
    /// @param nonce unique nonce.
    /// @param mac
    /// @note make sure that the nonce is unique each encryption and deception pair , never reuse nonces.
    /// (can be fixed with a synced counter)
    inline void encrypt(
        const key &key,
        std::span<uint8_t> data,
        const std::span<uint8_t> extra_data,
        const nonce& nonce,
        mac& mac)
    {
        const uint8_t *ad_ptr = extra_data.empty() ? nullptr : extra_data.data();

        const uint64_t ad_size = static_cast<uint64_t>(extra_data.size());
        const uint64_t text_size = static_cast<uint64_t>(data.size());

        crypto_aead_lock(
            data.data(),
            mac.bytes,
            key.bytes,
            nonce.bytes,
            ad_ptr,
            ad_size,
            data.data(),
            text_size
        );

        return;
    }

    /// tries to decrypt data, using KEY and NONCE.
    /// @param key shared key.
    /// @param data cypher text.
    /// @param extra_data extra given data.
    /// @param nonce shared unique nonce.
    /// @param mac mac, should be passed as plaintext.
    /// @return returns if successful decrypt.
    /// @note fail cases:
    /// @note - invalid key
    /// @note - wrong nonce
    /// @note - invalid mac (most likely messed with, so don't trust client)
    inline bool decrypt(
        const key &key,
        std::span<uint8_t> data,
        const std::span<uint8_t> extra_data,
        const nonce &nonce,
        const mac &mac)
    {
        const uint8_t *ad_ptr = extra_data.empty() ? nullptr : extra_data.data();
        const uint64_t ad_size = static_cast<uint64_t>(extra_data.size());
        const uint64_t text_size = static_cast<uint64_t>(data.size());

        const int r = crypto_aead_unlock(
            data.data(),
            mac.bytes,
            key.bytes,
            nonce.bytes,
            ad_ptr,
            ad_size,
            data.data(),
            text_size
        );

        return r == 0;
    }

    /// @brief used for abstracting handshakes.
    ///
    class handshake
    {
        bool _is_shared_valid = false;

        key _private{};
        key _public{};
        key _shared_secret{};

    public:
        /// @brief generates the private and public key.
        ///
        handshake()
        {
            crypto_rng::get_bytes(_private.bytes, sizeof(_private));
            _generate_public();
        }

        /// @brief public key getter.
        [[nodiscard]] key get_public() const
        {
            return _public;
        }

        /// @brief private key getter.
        [[nodiscard]] key get_private() const
        {
            return _private;
        }

        /// @brief shared key getter.
        /// @note only call if you know you called generate_shared_secret or checked validness:
        /// @code{.cpp}
        /// if ( hs.is_shared_valid() )
        /// {
        ///     key = hs.get_shared_secret();
        /// }
        /// @endcode
        [[nodiscard]] key get_shared_secret() const
        {
            return _shared_secret;
        }

        /// @brief generates the shared derived key.
        /// @param other_public the other handshakes public key.
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

        /// @brief checks if the shared key is valid, and if not call obj.generate_shared_secret(...);
        [[nodiscard]] bool is_shared_valid() const { return _is_shared_valid; }

    private:
        void _generate_public()
        {
            crypto_x25519_public_key( _public.bytes, _private.bytes );
        }
    };
}

#endif //BANKER_CRYPTER_HPP