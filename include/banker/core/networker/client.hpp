//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_CLIENT_HPP
#define BANKER_CLIENT_HPP

#include <functional>
#include <iostream>
#include <string>

#include "core/socket/socket.hpp"
#include "banker/debug_inspector.hpp"
#include "banker/core/crypto/crypter.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "buildt_in/base_packets.hpp"
#include "core/tcp/stream_handler.hpp"
#include "core/packet.hpp"

namespace banker::networker
{
    class client
    {
    public:
        using on_receive_callback = std::function<void(const packet&)>;

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
                return;
            }

            if (!_socket.set_blocking())
            {
                if (out) out << "Failed to set blocking mode" << std::endl;
            }

            auto _ = _send_packet({},base_packets::packet_type_to_server::request_public_key);
        }

        client(client&& other) noexcept
            : _socket(std::move(other._socket)) {}

        client& operator=(client&& other) noexcept { _socket = std::move(other._socket); return *this; }

        void on_receive(on_receive_callback callback)
        {
            _on_receive = std::move(callback);
        }

        void send(const packet& pkt) const
        {
            if (!_socket.is_valid())
            {
                std::cerr << "Cannot send — socket invalid\n";
                return;
            }

            if (_send_packet(pkt) <= 0)
            {
                std::cerr << "Send failed or connection closed\n";
            }
        }

        [[noreturn]] void run()
        {
            while (true)
            {
                tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        void tick()
        {
            _handle_incoming();
        }

    private:
        void _handle_incoming()
        {
            auto [packets, error] = networker::common::get_available_packets(
                _socket,
                _recv_buffer);

            if (error <= 0) return;

            for (auto& p : packets) _handle_packet(p);
        }

        void _handle_packet(packet& pkt)
        {
            const auto pt = pkt.read<base_packets::packet_type_from_server>();

            std::cout << "got " << base_packets::packet_type_from_server_to_string(pt) << " ";

            switch (pt)
            {
                case base_packets::packet_type_from_server::requested_public_key:
                {
                    _handshake.generate_shared_secret(pkt.read<crypter::key>());
                    std::cout
                        << "secret = " << format_bytes::to_hex(_handshake.get_shared_secret())
                        << std::endl;
                } break;
                case base_packets::packet_type_from_server::request_public_key:
                {
                    packet p{};
                    p.write(_handshake.get_public());
                    auto _ = _send_packet(p,base_packets::packet_type_to_server::requested_public_key);
                } break;
                default: ;
            }

            if (_on_receive && pt == base_packets::packet_type_from_server::user_defined) _on_receive(pkt);
        }

        [[nodiscard]] int _send_packet(
            const packet& pkt,
            base_packets::packet_type_to_server pt = base_packets::packet_type_to_server::user_defined) const
        {
            if (!_socket.is_valid()) return -1;

            std::cout << "send " << base_packets::packet_type_to_server_to_string(pt) << std::endl;

            return networker::common::send_packet_with_type(pkt, (uint8_t)pt, _socket);
        }

    private:
        socket _socket;
        crypter::handshake _handshake{};
        on_receive_callback _on_receive{};

        std::vector<uint8_t> _recv_buffer{};
    };
}

#endif //BANKER_CLIENT_HPP