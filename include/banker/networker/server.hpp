//
// Created by moosm on 11/3/2025.
//

#ifndef BANKER_SERVER_HPP
#define BANKER_SERVER_HPP

#include "core/networker.hpp"
#include "banker/debug_inspector.hpp"

#include <thread>
#include <iostream>
#include <vector>


namespace banker::networker
{
    class server
    {
    public:
        server() noexcept = default;
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

        void log_resv() const
        {
            std::vector<size_t> readable, writable;
            server::poll_sockets(_clients, readable, writable, 10);

            for (const auto idx : readable)
            {
                char buf[256] = {0};
                if (const int n = _clients[idx].recv(buf, sizeof(buf)-1); n > 0)
                    std::cout << "[server] received: " << buf << " from: " << _clients[idx].to_string() <<std::endl;
            }
        }

        void accept_backlog()
        {
            while (true)
            {
                auto c = _socket.accept();
                if (c.is_valid())
                {
                    std::cout << "[server] accepted " << c.to_string() << std::endl;
                    auto _ = c.set_blocking(false);
                    _clients.emplace_back(std::move(c));
                } else
                {
                    return;
                }
            }
        }

    private:
        socket _socket;
        std::vector<socket> _clients;
    public:
        /// @brief function to gather all the readable and writable sockets.
        /// @param sockets array of all accepted clients
        /// @param readable OUT array of all the readable socket idxs
        /// @param writable OUT array of all the writable socket idxs
        /// @param timeout_ms how long the selecting lasts
        static inline void poll_sockets(
            const std::vector<socket>& sockets,
            std::vector<size_t>& readable,
            std::vector<size_t>& writable,
            const int timeout_ms = 0)
        {
            readable.clear();
            writable.clear();

            if (sockets.empty()) return;

            fd_set read_fds, write_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);

            socket_t max_fd = 0;
            for (auto& s : sockets)
            {
                if (!s.is_valid()) continue;
                FD_SET(s.to_fd(), &read_fds);
                FD_SET(s.to_fd(), &write_fds);
                if (s.to_fd() > max_fd) max_fd = s.to_fd();
            }

            const timeval tv{ timeout_ms / 1000, (timeout_ms % 1000) * 1000 };

#ifdef _WIN32
            const int activity = select(0, &read_fds, &write_fds, nullptr, &tv);
#else
            const int activity = select(max_fd + 1, &read_fds, &write_fds, nullptr, &tv);
#endif

            if (activity <= 0) return;

            for (size_t i = 0; i < sockets.size(); ++i) {
                const auto& s = sockets[i];
                if (!s.is_valid()) continue;
                if (FD_ISSET(s.to_fd(), &read_fds))  readable.push_back(i);
                if (FD_ISSET(s.to_fd(), &write_fds)) writable.push_back(i);
            }
        }
    };
}

#endif //BANKER_SERVER_HPP