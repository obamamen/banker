//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_CLIENT_HPP
#define BANKER_CLIENT_HPP

#include <iostream>
#include <string>

#include "core/socket.hpp"
#include "../debug_inspector.hpp"
#include "banker/crypto/format_bytes.hpp"
#include "core/packet.hpp"

namespace banker::networker
{
    class client
    {
    public:
        /// @brief if the underlying socket is valid
        [[nodiscard]] bool is_valid() const noexcept { return _socket.is_valid(); }

        client() noexcept = default;
        ~client() noexcept = default;

        /// @brief client ctor
        /// @param host ip address host
        /// @param port port host
        /// @param out where to debug info to, set to nullptr to disable
        client(const std::string& host, const u_short port, std::ostream& out = std::cerr)
        {
            if (!_socket.create())
            {
                if (out) out << "Failed to create client socket" << std::endl;
            }

            if (!_socket.connect(host, port))
            {
                if (out) out << "Failed to connect client socket " << INSPECT(
                    INSPECT_V(host),
                    INSPECT_V(port)
                ) << std::endl;
            }
        }

        client(client&& other) noexcept
            : _socket(std::move(other._socket)) {}

        client& operator=(client&& other) noexcept { _socket = std::move(other._socket); return *this; }

        void send(const packet& pkt) const
        {
            if (!_socket.is_valid())
            {
                std::cerr << "Cannot send — socket invalid\n";
                return;
            }

            const auto serialized = pkt.serialize();
            const int sent = _socket.send(serialized.data(), serialized.size());
            if (sent <= 0)
            {
                std::cerr << "Send failed or connection closed\n";
            }
        }

    private:
        socket _socket;
    };
}

#endif //BANKER_CLIENT_HPP