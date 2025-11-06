//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_CRYPTO_SOCKET_HPP
#define BANKER_CRYPTO_SOCKET_HPP

#include <cstdint>
#include <vector>

#include "banker/crypto/crypter.hpp"
#include "banker/networker/core/socket.hpp"

namespace banker
{
    /// @brief a socket wrapper around the TCP protocol, with crypter::handshake as safety.
    ///
    class crypto_socket
    {
    public:
        crypto_socket();
        ~crypto_socket();

        crypto_socket(const crypto_socket&) = delete;
        crypto_socket& operator=(const crypto_socket&) = delete;

        crypto_socket(crypto_socket&) = delete;
        crypto_socket& operator=(crypto_socket&) = delete;

        crypto_socket(crypto_socket&& other) noexcept
            : _socket{std::move(other._socket)}
        { }

        crypto_socket& operator=(crypto_socket&& other) noexcept
        {
            if (this != &other)
            {
                _socket = std::move(other._socket);
            }
            return *this;
        }

    private:
        networker::socket _socket{};
        crypter::handshake _handshake{};
        uint64_t _counter{};
        std::vector<uint8_t> _buffer;
    };
}

#endif //BANKER_CRYPTO_SOCKET_HPP