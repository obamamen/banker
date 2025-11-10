//
// Created by moosm on 11/6/2025.
//

#ifndef BANKER_CRYPTO_SOCKET_HPP
#define BANKER_CRYPTO_SOCKET_HPP

#include <cstdint>
#include <deque>
#include <format>
#include <vector>

#include "banker/crypto/crypter.hpp"
#include "banker/networker/core/socket/socket.hpp"
#include "banker/networker/core/packet.hpp"

#include "banker/networker/common/stream_handler.hpp"

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

    public:
        crypto_channel();
        ~crypto_channel();

        explicit crypto_channel(networker::socket&& socket)
        {
            _socket = std::move(socket);
        }

        crypto_channel(crypto_channel&) = delete;
        crypto_channel& operator=(crypto_channel&) = delete;

        crypto_channel(crypto_channel&& other) noexcept
            : _socket{std::move(other._socket)}
        { }

        crypto_channel& operator=(crypto_channel&& other) noexcept
        {
            if (this != &other)
            {
                _socket = std::move(other._socket);
            }
            return *this;
        }

        bool connect(const std::string& host, const uint16_t port) const
        {
            return _socket.connect(host, port);
        }

        void initiate_handshake();

        void secure_send(networker::packet);

        void send(networker::packet)
        {
            std::vector<uint8_t> buffer;
        }

        networker::packet receive()
        {
            auto packet = networker::common::get_packet_from_stream(_buffer);
            return packet;
        }

        void tick()
        {
            networker::common::stream_socket(_socket,_buffer);
            while (true)
            {
                auto p = networker::common::get_packet_from_stream(_buffer);
                if (!p.is_valid()) break;

                _handle_packet(p);
            }
        }

    private:
        void _handle_packet(const networker::packet &packet)
        {
            _received_packets.push_back(packet);
            _counter++;
        }

        void _send(const std::vector<uint8_t>& binary)
        {
            const auto _ = _socket.send(
                binary.data(),
                binary.size());

            if (_ == binary.size())
            {
                _counter++;
            }
        }

        networker::socket _socket{};
        crypter::handshake _handshake{};
        uint64_t _counter{0};
        std::vector<uint8_t> _buffer{};
        std::deque<networker::packet> _received_packets{};
    };
}

#endif //BANKER_CRYPTO_SOCKET_HPP