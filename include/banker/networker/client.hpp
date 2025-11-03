//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_CLIENT_HPP
#define BANKER_CLIENT_HPP
#include <iostream>
#include <string>

#include "core/socket.hpp"
#include "../debug_inspector.hpp"

namespace banker::networker
{
    class client
    {
    public:
        [[nodiscard]] bool is_valid() const noexcept { return _socket.is_initialized(); }

        client() noexcept = default;
        ~client() noexcept = default;

        client(const std::string& host, const u_short port, std::ostream& out = std::cerr)
        {
            if (!_socket.create())
            {
                out << "Failed to create client socket" << std::endl;
            }

            if (!_socket.connect(host, port))
            {
                out << "Failed to connect client socket " << INSPECT(
                    INSPECT_V(host),
                    INSPECT_V(port)
                ) << std::endl;
            }

            const std::string message = "Hello World!";
            _socket.send(message.c_str(), message.size());
        }

        client(client&& other) noexcept
            : _socket(std::move(other._socket)) {}
        client& operator=(client&& other) noexcept { _socket = std::move(other._socket); return *this; }

    private:
        socket _socket;
    };
}

#endif //BANKER_CLIENT_HPP