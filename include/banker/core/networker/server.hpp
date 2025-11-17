/* ================================== *\
 @file     server.hpp
 @project  banker
 @author   moosm
 @date     11/17/2025
*\ ================================== */

#ifndef BANKER_SERVER_HPP
#define BANKER_SERVER_HPP

#include <cstdint>
#include <functional>

#include "base_packet_types.hpp"
#include "core/socket/polling.hpp"
#include "core/tcp/packet_stream_core.hpp"
#include "crypto/crypto_core.hpp"

namespace banker::networker
{
    class server
    {
    public:
        using client = uint64_t;

        using on_connect_callback = std::function<void(client)>;
        using on_disconnect_callback = std::function<void(client)>;
        using on_receive_callback = std::function<void(client, packet)>;
        using on_error_callback = std::function<void(client, const std::string& msg)>;
        using on_internal_message_callback = std::function<void(const std::string&)>;

    private:
        struct internal_client
        {
            client id;
            socket socket;
            packet_stream_core packet_stream;
            crypto_core crypto_core;
            crypter::handshake handshake;
        };

    public:
        server()                            = delete;
        ~server()                           = default;

        server(const server&)               = delete;
        server& operator=(const server&)    = delete;

        server(server&&)                    = default;
        server& operator=(server&&)         = default;

        explicit server(const uint16_t port)
        {
            bool valid = false;
            socket& host_socket = _host;

            valid = packet_stream_host_core::create(host_socket);
            if (!valid) std::cerr << "Failed to create server socket" << std::endl;

            valid = packet_stream_host_core::bind(host_socket, port);
            if (!valid) std::cerr << "Failed bind" << std::endl;

            valid = packet_stream_host_core::listen(host_socket);
            if (!valid) std::cerr << "Failed init listen" << std::endl;
        }

        void set_on_connect(
            on_connect_callback callback)
        {
            _on_connect = std::move( callback );
        }

        void set_on_disconnect(
            on_disconnect_callback callback)
        {
            _on_disconnect = std::move( callback );
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

        void disconnect(
            const client id)
        {
            _to_disconnect.push_back(id);
        }

        void send_packet(
            client& id,
            packet& user_packet)
        {
            networker::packet main_packet{};
            crypter::mac hmac{};

            internal_client& ic = _internal_clients[_client_to_index[id]];

            crypter::encrypt(
                ic.handshake.get_shared_secret(),
                user_packet.get_remaining_data(),
                {},
                ic.crypto_core.generate_outgoing_nonce(),
                hmac);

            main_packet.write(base_packet_types::to_server::user_defined);
            main_packet.write(hmac);
            main_packet.write(user_packet);
            ic.packet_stream.buffer(main_packet);
        }

        void tick()
        {
            _accept();

            std::vector<socket*> socket_ptrs;
            socket_ptrs.reserve(1 + _internal_clients.size());
            socket_ptrs.push_back(&_host);

            for (auto& c : _internal_clients)
                socket_ptrs.push_back(&c.socket);

            auto results = socket_poller::poll(
                socket_ptrs,
                10);

            for (auto& r : results)
            {
                if (r.index == 0)
                {
                    _accept();
                }
                else
                {
                    const size_t client_idx = r.index - 1;
                    internal_client& client = _internal_clients[client_idx];

                    if (r.readable)
                    {
                        client.packet_stream.tick_receive(client.socket);
                    }

                    if (r.writable)
                    {
                        const auto c = client.packet_stream.flush_send_buffer(client.socket);
                        client.crypto_core.increment_outgoing(c);
                    }

                    packet packet = client.packet_stream.receive_packet();
                    if (packet.is_valid())
                    {
                        _handle_packet(packet,client);
                        client.crypto_core.increment_incoming();
                    }
                }
            }


        }

    private:
        void _handle_packet(
            packet& packet,
            internal_client& client)
        {
            std::cout << "PACKET: " << format_bytes::to_hex(packet.get_data()) << std::endl;
            bool valid = false;

            const auto type = packet.read< base_packet_types::to_server >(&valid);
            if (!valid) return;

            if (type == base_packet_types::to_server::handshake)
            {
                const auto p_key = packet.read< crypter::key >(&valid);
                if (!valid) return;
                client.handshake.generate_shared_secret(p_key);
                networker::packet response;
                response.write(base_packet_types::from_server::handshake_response);
                response.write(client.handshake.get_public());
                client.packet_stream.buffer(response);
                std::cout << "Derived key: " << format_bytes::to_hex(client.handshake.get_shared_secret()) << std::endl;
            }

            if (type == base_packet_types::to_server::user_defined)
            {
                if ( client.handshake.is_shared_valid() )
                {
                    crypter::mac hmac =
                        packet.read< crypter::mac >(&valid);
                    if (!valid) return;

                    auto user_packet =
                        packet.read<networker::packet>(&valid);
                    if (!valid) return;

                    bool d = crypter::decrypt(
                        client.handshake.get_shared_secret(),
                        user_packet.get_data(),
                        {},
                        client.crypto_core.generate_incoming_nonce(),
                        hmac);

                    //std::cout << "d: " << d << std::endl;

                    if (_on_receive) _on_receive(client.id, std::move(user_packet));
                } else
                {
                    throw std::runtime_error("Not yet set shared secret");
                }
            }
        }

        void _accept()
        {
            socket new_sock = packet_stream_host_core::accept_incoming(
                _host);

            if (!new_sock.is_valid()) return;

            std::cout << "accepted new client " << new_sock.to_fd() << std::endl;

            const client id = _next_client++;
            _internal_clients.push_back({
                id,
                std::move( new_sock ),
                {},
                {},
                {}});

            _client_to_index[id] = _internal_clients.size() - 1;
        }

    private:
        socket _host;

        client _next_client{0};

        std::vector<internal_client> _internal_clients{};
        std::unordered_map<client, size_t> _client_to_index{};
        std::vector<client> _to_disconnect{};

        on_connect_callback _on_connect = {};
        on_disconnect_callback _on_disconnect = {};
        on_receive_callback _on_receive = {};
        on_error_callback _on_error = {};
        on_internal_message_callback _on_internal_message = {};
    };
}

#endif //BANKER_SERVER_HPP