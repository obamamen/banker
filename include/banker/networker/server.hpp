//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_SERVER_HPP
#define BANKER_SERVER_HPP

#include <functional>

#include "core/networker.hpp"
#include "banker/debug_inspector.hpp"

#include <thread>
#include <iostream>
#include <map>
#include <ranges>
#include <vector>

#include "banker/common/formatting/header.hpp"
#include "banker/crypto/crypter.hpp"
#include "buildt_in/base_packets.hpp"
#include "core/packet.hpp"


namespace banker::networker
{
    class server
    {
    public:
        using client_id = size_t;

        using on_connect_callback = std::function<void(client_id)>;
        using on_disconnect_callback = std::function<void(client_id)>;
        using on_receive_callback = std::function<void(client_id, const packet&)>;
        using on_error_callback = std::function<void(client_id, const std::string&)>;
        using on_internal_message_callback = std::function<void(const std::string&)>;

    private:
        struct internal_client
        {
            socket client_socket{};
            crypter::handshake handshake{};
            std::vector<uint8_t> recv_buffer;
            bool connected = false;

            void connect(socket socket)
            {
                client_socket = std::move(socket);
                connected = true;
            }

            void disconnect()
            {
                connected = false;
                if (client_socket.close() == socket::socket_error)
                {
                    std::cerr << "Failed to close socket" << std::endl;
                }
            }
        };
    public:
        server() noexcept = delete;
        ~server() noexcept = default;

        /// @brief server ctor.
        /// @param port which port to host on.
        /// @param ip what ip to host on. ("127.0.0.1")
        /// @param out where to debug info to, set to nullptr to disable.
        explicit server(const u_short port, const std::string& ip = "127.0.0.1", std::ostream& out = std::cerr)
            : _ip(ip), _port(port)
        {
            if (!_socket.create())
            {
                out << "Failed to create server socket" << std::endl;
            }

            if (!_socket.bind(port, ip))
            {
                out << "Failed to bind server socket " << INSPECT(
                    INSPECT_V(port)
                ) << std::endl;
            }

            if (!_socket.listen(256))
            {
                out << "Failed to turn on server listen " << INSPECT(
                    INSPECT_V(port)
                ) << std::endl;
            }

            if (!_socket.set_blocking(false))
            {
                out << "Failed to turn on server non_blocking " << INSPECT(
                    INSPECT_V(port)
                ) << std::endl;
            }
        }

        void set_on_connect(on_connect_callback callback)
        {
            _on_connect = std::move(callback);
        }

        void set_on_disconnect(on_disconnect_callback callback)
        {
            _on_disconnect = std::move(callback);
        }

        void set_on_receive(on_receive_callback callback)
        {
            _on_receive = std::move(callback);
        }

        void set_on_error(on_error_callback callback)
        {
            _on_error = std::move(callback);
        }

        void set_on_internal_message(on_internal_message_callback callback)
        {
            _on_internal_message = std::move(callback);
        }

        void disconnect_client(const client_id client)
        {
            _disconnect_client(client);
        }

        [[nodiscard]] size_t client_count() const
        {
            size_t count = 0;
            for (const auto &client: _clients | std::views::values)
            {
                if (client.connected) { ++count; }
            }
            return count;
        }

        [[noreturn]] void run()
        {
            constexpr int width = 60;
            constexpr char sides = '|';
            common::formatting::print_divider(width,'=',"",sides);
            common::formatting::print_divider(width, ' ',
                common::formatting::format("server starting ", _ip, ":", _port),sides);
            common::formatting::print_divider(width,'-',"",sides);
            common::formatting::print_divider(width, ' '," ... ", sides);
            common::formatting::print_divider(width,'=',"",sides);
            while (true)
            {
                this->tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        void tick()
        {
            _accept_backlog();
            _process_clients();
        }

    private:
        bool _send_packet(
            const client_id client,
            const packet& pkt,
            base_packets::packet_type_from_server pt = base_packets::packet_type_from_server::user_defined)
        {
            if (!_clients.contains(client)) return false;
            const auto &c = _clients[client];
            if (!c.connected) return false;

            packet wrapper = {};
            wrapper.write(static_cast<uint8_t>(pt));
            wrapper.insert_bytes(pkt.get_data());

            // std::cout << "send " << base_packets::packet_type_from_server_to_string(pt) <<
            //     " to " <<  client <<std::endl;

            const auto serialized = wrapper.serialize();
            const int sent = c.client_socket.send(serialized.data(), serialized.size());
            return sent == static_cast<int>(serialized.size());
        }

        void _process_clients()
        {
            std::vector<size_t> readable;
            std::vector<size_t> writable;
            _poll_sockets(readable, writable, 0);

            for (auto id : readable)
            {
                auto &client = _clients[id];
                if (!client.connected) continue;

                uint8_t buffer[4096];
                const int bytes = client.client_socket.recv(buffer, sizeof(buffer));
                if (bytes <= 0)
                {
                    _disconnect_client(id);
                    continue;
                }

                // if (bytes < 0)
                // {
                //     _disconnect_client(id);
                //     continue;
                // }

                client.recv_buffer.insert(client.recv_buffer.end(), buffer, buffer + bytes);

                while (true)
                {
                    auto pkt = packet::deserialize(client.recv_buffer);

                    if (pkt.get_data().empty())
                    {
                        break;
                    }

                    _handle_packet(id, pkt);
                }
            }
        }

        void _handle_packet(const client_id from, packet& pkt)
        {
            const auto pt = pkt.read<base_packets::packet_type_to_server>();

            // std::cout <<
            //     "got " << base_packets::packet_type_to_server_to_string(pt)
            //     << " from " << from << std::endl;

            switch (pt)
            {
                case base_packets::packet_type_to_server::request_public_key:
                {
                    packet p{};
                    const crypter::handshake& handshake = _clients.at(from).handshake;
                    p.write(handshake.get_public());
                    _send_packet(from,p,base_packets::packet_type_from_server::requested_public_key);
                    _send_packet(from,{},base_packets::packet_type_from_server::request_public_key);
                } break;

                case base_packets::packet_type_to_server::requested_public_key:
                {
                    _clients.at(from).handshake.generate_shared_secret(pkt.read<crypter::key>());
                    std::stringstream ss;

                    if (_on_internal_message) ss << "established handshake with " << from <<
                        " [" <<
                            format_bytes::to_hex(_clients.at(from).handshake.get_shared_secret(),"-",4) <<
                        "] ";

                    if (_on_internal_message) _on_internal_message(ss.str());
                } break;
                default: ;
            }

            if (_on_receive && pt == base_packets::packet_type_to_server::user_defined) _on_receive(from, pkt);
        }

        void _accept_backlog()
        {
            while (true)
            {
                auto c = _socket.accept();
                if (c.is_valid())
                {
                    auto _ = c.set_blocking(false);
                    _add_client(c);
                } else
                {
                    return;
                }
            }
        }

        void _add_client(socket& socket)
        {
            _clients[_next_client_id] = {};
            _clients[_next_client_id].connect(std::move(socket));
            _clients[_next_client_id].handshake = crypter::handshake();
            if (_on_connect)
            {
                _on_connect(_next_client_id);
            }
            _next_client_id = (_next_client_id + 1);
        }

        void _disconnect_client(const client_id client)
        {
            if (!_clients.contains(client))
            {
                return;
            }

            _clients[client].disconnect();
            _clients.erase(client);

            if (_on_disconnect)
            {
                _on_disconnect(client);
            }
        }

        void _poll_sockets(
            std::vector<size_t>& readable,
            std::vector<size_t>& writable,
            const int timeout_ms = 0)
        {
            readable.clear();
            writable.clear();

            fd_set read_fds, write_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);

            socket_t max_fd = 0;

            for (const auto& [id, client] : _clients)
            {
                if (!client.connected || !client.client_socket.is_valid())
                {
                    continue;
                }

                socket_t fd = client.client_socket.to_fd();
                FD_SET(fd, &read_fds);
                FD_SET(fd, &write_fds);

                if (fd > max_fd)
                {
                    max_fd = fd;
                }
            }

            const timeval tv{ timeout_ms / 1000, (timeout_ms % 1000) * 1000 };

#ifdef _WIN32
            const int activity = select(0, &read_fds, &write_fds, nullptr, &tv);
#else
            const int activity = select(max_fd + 1, &read_fds, &write_fds, nullptr, &tv);
#endif

            if (activity <= 0)
            {
                return;
            }

            for (const auto& [id, client] : _clients)
            {
                if (!client.connected || !client.client_socket.is_valid())
                {
                    continue;
                }

                socket_t fd = client.client_socket.to_fd();

                if (FD_ISSET(fd, &read_fds))
                {
                    readable.push_back(id);
                }

                if (FD_ISSET(fd, &write_fds))
                {
                    writable.push_back(id);
                }
            }
        }

    private:
        socket _socket;
        std::map<client_id, internal_client> _clients;
        client_id _next_client_id = 0;

        on_connect_callback _on_connect = {};
        on_disconnect_callback _on_disconnect = {};
        on_receive_callback _on_receive = {};
        on_error_callback _on_error = {};
        on_internal_message_callback _on_internal_message = {};

        const std::string _ip = "";
        const int _port = -1;
    };
}

#endif //BANKER_SERVER_HPP