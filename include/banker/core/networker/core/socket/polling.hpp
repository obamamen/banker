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

#include "banker/core/networker/core/socket/socket.hpp"

namespace banker::networker
{
    class poll_group
    {
    public:
        struct result
        {
            bool readable       {false};
            bool writable       {false};
            bool error          {false};
            bool disconnected   {false};
        };

    public:
        void reserve(const size_t size)
        {
            _fds.reserve(size);
        }

        void add(const socket& socket)
        {
#ifdef _WIN32
            _fds.emplace_back(
                socket.to_fd(),
                POLLRDNORM | POLLWRNORM,
                0);
#endif
        }

        void reset()
        {
#ifdef _WIN32
            _read_offset = 0;
            _fds.clear();
#endif
        }

        void poll(const int timeout = 0)
        {
#ifdef _WIN32
            _read_offset = 0;

            if (_fds.empty()) {
                std::cout << "Warning: polling with 0 sockets\n";
                return;  // Don't call WSAPoll with empty array
            }

            for (int i = 0; i < _fds.size(); ++i)
            {
                std::cout << " -  ["<<i<<"]"<<_fds[i].fd << "\n";
            }

            std::cout << "Polling " << _fds.size() << " sockets\n";
            for (size_t i = 0; i < _fds.size(); ++i) {
                std::cout << "  [" << i << "] fd=" << _fds[i].fd
                          << " events=" << _fds[i].events << "\n";
            }

            int result = WSAPoll(_fds.data(), static_cast<ULONG>(_fds.size()), static_cast<INT>(timeout));
            if (result == SOCKET_ERROR) {
                int error = WSAGetLastError();
                std::cout << "WSAPoll ERROR: " << error << "\n";
            }
#endif
        }

        /// @brief
        /// @param result
        /// @return index of socket or -1 if end of socket
        int next_result(
            result& result)
        {
#ifdef _WIN32
            if (_read_offset >= _fds.size()) return -1;

            result =
            {
                (_fds[_read_offset].revents & POLLIN) != 0,
                (_fds[_read_offset].revents & POLLOUT) != 0,
                (_fds[_read_offset].revents & POLLERR) != 0,
                (_fds[_read_offset].revents & POLLHUP) != 0,
            };

            return _read_offset++;
#endif
        }

    private:
#ifdef _WIN32
        std::vector<WSAPOLLFD> _fds{};
        uint16_t _read_offset{0};
#else
#endif
    };
}


#endif //BANKER_POLLING_HPP