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

    private:
        struct internal_client
        {
            socket client_socket{};
            uint8_t local_private_k[32]{};
            uint8_t used_public_k[32]{};
            uint8_t shared_private_k[32]{};
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

        /// @brief server ctor
        /// @param port which port to host on
        /// @param out where to debug info to, set to nullptr to disable
        explicit server(const u_short port, std::ostream& out = std::cerr)
        {
            if (!_socket.create())
            {
                out << "Failed to create server socket" << std::endl;
            }

            if (!_socket.bind(port))
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
            while (true)
            {
                this->tick();
            }
        }

        void tick()
        {
            _accept_backlog();
            _process_clients();
        }

    private:
        bool _send_packet(const client_id client, const packet& pkt)
        {
            if (!_clients.contains(client)) return false;
            const auto &c = _clients[client];
            if (!c.connected) return false;

            const auto serialized = pkt.serialize();
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
                if (bytes == 0)
                {
                    _disconnect_client(id);
                    continue;
                }

                if (bytes < 0)
                {
                    if (_on_error) _on_error(id, "recv() failed");
                    _disconnect_client(id);
                    continue;
                }

                client.recv_buffer.insert(client.recv_buffer.end(), buffer, buffer + bytes);

                while (true)
                {
                    auto pkt = packet::deserialize(client.recv_buffer);

                    if (pkt.get_data().empty())
                    {
                        break;
                    }

                    if (_on_receive)
                        _on_receive(id, pkt);
                }
            }
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
    };
}

#endif //BANKER_SERVER_HPP