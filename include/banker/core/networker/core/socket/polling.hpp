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
        uint64_t index      {0};
        bool readable       {false};
        bool writable       {false};
        bool error          {false};
        bool disconnected   {false};
    };

    class poll_group
    {
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
                POLLIN | POLLOUT | POLLERR | POLLHUP,
                0);
#endif
        }

        void reset()
        {
            _read_offset = 0;
            _fds.clear();
        }

        void poll(const int timeout = 0)
        {
#ifdef _WIN32
            WSAPoll(_fds.data(),static_cast<ULONG>(_fds.size()),static_cast<INT>(timeout));
#endif
        }

        bool next_result(socket_poll_result& result)
        {
#ifdef _WIN32
            if (_read_offset >= _fds.size()) return false;
            result =
            {
                _read_offset,
                (_fds[_read_offset].revents & POLLIN) != 0,
                (_fds[_read_offset].revents & POLLOUT) != 0,
                (_fds[_read_offset].revents & POLLERR) != 0,
                (_fds[_read_offset].revents & POLLHUP) != 0,
            };

            _read_offset++;
            return true;
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