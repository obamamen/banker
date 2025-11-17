/* ================================== *\
 @file     client.hpp
 @project  banker
 @author   moosm
 @date     11/17/2025
*\ ================================== */

#ifndef BANKER_CLIENT_HPP
#define BANKER_CLIENT_HPP

#include <cstdint>
#include <functional>

#include "banker/debug_inspector.hpp"
#include "base_packet_types.hpp"
#include "banker/core/crypto/format_bytes.hpp"
#include "core/socket/polling.hpp"
#include "core/tcp/packet_stream_core.hpp"
#include "crypto/crypto_core.hpp"

namespace banker::networker
{
    class client
    {
    public:
        using on_receive_callback = std::function<void(packet)>;
        using on_disconnect_callback = std::function<void()>;
        using on_error_callback = std::function<void(const std::string& msg)>;
        using on_internal_message_callback = std::function<void(const std::string&)>;

        client()                            = default;
        ~client()                           = default;

        client(const client&)               = delete;
        client& operator=(const client&)    = delete;

        client(client&&)                    = default;
        client& operator=(client&&)         = default;

        client(
            const std::string& host,
            const uint16_t port,
            std::ostream& out = std::cerr)
        {
            bool valid = false;

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
        }

        void set_on_receive(
            on_receive_callback callback)
        {
            _on_receive = std::move( callback );
        }

        void set_on_error(
            on_error_callback callback)
        {
            _on_error = std::move( callback );
        }

        void set_on_internal_message(
            on_internal_message_callback callback)
        {
            _on_internal_message = std::move( callback );
        }

        void initiate_handshake()
        {
            packet handshake_pkt{};
            handshake_pkt.write(base_packet_types::to_server::handshake);
            handshake_pkt.write(_handshake.get_public());
            _stream.buffer(handshake_pkt);
        }

        bool allowed_to_send()
        {
            return _handshake.is_shared_valid();
        }

        void tick()
        {
            _stream.tick_receive(_socket);
            packet packet = _stream.receive_packet();
            if (packet.is_valid())
            {
                _handle_packet(packet);
                _crypto.increment_incoming();
            }

            const auto c = _stream.flush_send_buffer(_socket);
            _crypto.increment_outgoing(c);
        }

        void send_packet(
            packet& user_packet)
        {
            networker::packet main_packet{};
            crypter::mac hmac{};

            std::cout << "User packet: " << format_bytes::to_hex(user_packet.get_data()) << std::endl;

            crypter::encrypt(
                _handshake.get_shared_secret(),
                user_packet.get_remaining_data(),
                {},
                _crypto.generate_outgoing_nonce(),
                hmac);

            main_packet.write(base_packet_types::to_server::user_defined);
            main_packet.write(hmac);
            main_packet.write(user_packet);
            _stream.buffer(main_packet);
        }

    private:
        void _handle_packet(
            packet& packet)
        {
            bool valid = false;

            const auto type = packet.read< base_packet_types::from_server >(&valid);
            if (!valid) return;

            if (type == base_packet_types::from_server::handshake_response)
            {
                const auto p_key = packet.read< crypter::key >(&valid);
                if (!valid) return;
                _handshake.generate_shared_secret(p_key);
                std::cout << "Derived key: " << format_bytes::to_hex(_handshake.get_shared_secret()) << std::endl;
            }

            if (type == base_packet_types::from_server::user_defined)
            {
                if ( _handshake.is_shared_valid() )
                {
                    auto hmac =
                        packet.read< crypter::mac >(&valid);
                    if (!valid) return;

                    auto user_packet =
                        packet.read<networker::packet>(&valid);
                    if (!valid) return;

                    crypter::decrypt(
                        _handshake.get_shared_secret(),
                        user_packet.get_data(),
                        {},
                        _crypto.generate_incoming_nonce(),
                        hmac);

                    if (_on_receive) _on_receive(std::move(user_packet));
                } else
                {
                    throw std::runtime_error("Not yet set shared secret");
                }
            }
        }
    private:
        packet_stream_core _stream{};
        crypto_core _crypto{};
        crypter::handshake _handshake{};
        socket _socket{};

        on_receive_callback _on_receive = {};
        on_error_callback _on_error = {};
        on_internal_message_callback _on_internal_message = {};
    };
}

#endif //BANKER_CLIENT_HPP