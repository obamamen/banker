/* ================================== *\
 @file     polling.hpp
 @project  banker
 @author   moosm
 @date     11/17/2025
*\ ================================== */

#ifndef BANKER_POLLING_HPP
#define BANKER_POLLING_HPP

#include <vector>
#include <span>
#include <chrono>
#include <algorithm>

#include "socket.hpp"

namespace banker::networker
{
    struct socket_poll_result
    {
        uint64_t index{ 0 };
        bool readable{ false };
        bool writable{ false };
        bool error{ false };
    };

    class socket_poller
    {
    public:
        static std::vector<socket_poll_result> poll(
            const std::span<socket> sockets,
            const uint64_t timeout_ms = 0)
        {
            std::vector<socket_poll_result> results;
            if (sockets.empty()) return results;

#ifdef _WIN32
            std::vector<WSAPOLLFD> fds;
            std::vector<size_t> indices;
            fds.reserve(sockets.size());
            indices.reserve(sockets.size());

            for (size_t i = 0; i < sockets.size(); ++i)
            {
                const auto& s = sockets[i];
                if (!s.is_valid()) continue;

                WSAPOLLFD fd{};
                fd.fd = s.to_fd();
                fd.events = POLLIN | POLLOUT;
                fd.revents = 0;

                fds.push_back(fd);
                indices.push_back(i);
            }

            int ret = WSAPoll(fds.data(), static_cast<ULONG>(fds.size()), static_cast<int>(timeout_ms));
            if (ret <= 0) return results;

            results.reserve(fds.size());
            for (size_t i = 0; i < fds.size(); ++i)
            {
                results.push_back({
                    (indices[i]),
                    (fds[i].revents & POLLIN) != 0,
                    (fds[i].revents & POLLOUT) != 0,
                    (fds[i].revents & POLLERR) != 0
                });
            }
#else
            fd_set readfds, writefds, exceptfds;
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            int max_fd = 0;
            std::vector<size_t> valid_indices;
            valid_indices.reserve(sockets.size());

            for (size_t i = 0; i < sockets.size(); ++i)
            {
                const auto& s = sockets[i];
                if (!s.is_valid()) continue;

                auto fd = s.to_fd();
                FD_SET(fd, &readfds);
                FD_SET(fd, &writefds);
                FD_SET(fd, &exceptfds);
                if (fd > max_fd) max_fd = fd;

                valid_indices.push_back(i);
            }

            timeval tv{};
            tv.tv_sec  = static_cast<long>(timeout_ms / 1000);
            tv.tv_usec = static_cast<long>((timeout_ms % 1000) * 1000);

            int ret = ::select(max_fd + 1, &readfds, &writefds, &exceptfds, &tv);
            if (ret <= 0) return results;

            results.reserve(valid_indices.size());
            for (auto idx : valid_indices)
            {
                const auto& s = sockets[idx];
                results.push_back({
                    (idx),
                    FD_ISSET(s.to_fd(), &readfds),
                    FD_ISSET(s.to_fd(), &writefds),
                    FD_ISSET(s.to_fd(), &exceptfds)
                });
            }
#endif
            return results;
        }

        static std::vector<socket_poll_result> poll(
            const std::span<socket*> sockets,
            const uint64_t timeout_ms = 0)
        {
            std::vector<socket_poll_result> results;
            if (sockets.empty()) return results;

#ifdef _WIN32
            std::vector<WSAPOLLFD> fds;
            std::vector<size_t> indices;
            fds.reserve(sockets.size());
            indices.reserve(sockets.size());

            for (size_t i = 0; i < sockets.size(); ++i)
            {
                auto* s = sockets[i];
                if (!s || !s->is_valid()) continue;

                WSAPOLLFD fd{};
                fd.fd = s->to_fd();
                fd.events = POLLIN | POLLOUT;
                fd.revents = 0;

                fds.push_back(fd);
                indices.push_back(i);
            }

            int ret = WSAPoll(fds.data(), static_cast<ULONG>(fds.size()), static_cast<int>(timeout_ms));
            if (ret <= 0) return results;

            results.reserve(fds.size());
            for (size_t i = 0; i < fds.size(); ++i)
            {
                results.push_back({
                    (indices[i]),
                    (fds[i].revents & POLLIN) != 0,
                    (fds[i].revents & POLLOUT) != 0,
                    (fds[i].revents & POLLERR) != 0
                });
            }

#else // Unix
            fd_set readfds, writefds, exceptfds;
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            int max_fd = 0;
            std::vector<size_t> valid_indices;
            valid_indices.reserve(sockets.size());

            for (size_t i = 0; i < sockets.size(); ++i)
            {
                auto* s = sockets[i];
                if (!s || !s->is_valid()) continue;

                auto fd = s->to_fd();
                FD_SET(fd, &readfds);
                FD_SET(fd, &writefds);
                FD_SET(fd, &exceptfds);
                if (fd > max_fd) max_fd = fd;

                valid_indices.push_back(i);
            }

            timeval tv{};
            tv.tv_sec  = static_cast<long>(timeout_ms / 1000);
            tv.tv_usec = static_cast<long>((timeout_ms % 1000) * 1000);

            int ret = ::select(max_fd + 1, &readfds, &writefds, &exceptfds, &tv);
            if (ret <= 0) return results;

            results.reserve(valid_indices.size());
            for (auto idx : valid_indices)
            {
                auto* s = sockets[idx];
                results.push_back({
                    (idx),
                    FD_ISSET(s->to_fd(), &readfds),
                    FD_ISSET(s->to_fd(), &writefds),
                    FD_ISSET(s->to_fd(), &exceptfds)
                });
            }
#endif
            return results;
        }
    };
}


#endif //BANKER_POLLING_HPP