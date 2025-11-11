//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_CRYPTO_SOCKET_HPP
#define BANKER_CRYPTO_SOCKET_HPP

#include <cstdint>
#include <deque>
#include <format>
#include <vector>

#include "../core/tcp/stream_handler.hpp"
#include "banker/core/networker/core/packet.hpp"
#include "banker/core/crypto/crypter.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/core/networker/core/packet.hpp"

#include "banker/core/networker/core/tcp/stream_handler.hpp"

namespace banker
{

    /// @brief a socket wrapper around the TCP protocol, with crypter::handshake as safety.
    ///
    class crypto_channel
    {
        enum class packet_type : uint8_t
        {
            error               = 0,
            handshake_offer     = 1,
            handshake_answer    = 2,
            encrypted           = 3,
            plaintext           = 4,
        };

        // TODO

    };
}

#endif //BANKER_CRYPTO_SOCKET_HPP