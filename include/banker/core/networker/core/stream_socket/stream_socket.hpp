/* ================================== *\
 @file     stream_socket.hpp
 @project  banker
 @author   moosm
 @date     12/12/2025
*\ ================================== */

#ifndef BANKER_STREAM_SOCKET_HPP
#define BANKER_STREAM_SOCKET_HPP
#include <cstdint>

#include "stream_socket_core.hpp"
#include "banker/core/networker/core/socket/socket.hpp"
#include "banker/shared/compat.hpp"

namespace banker::networker
{
    class stream_socket
    {
    public:
        class acceptor;

    public:
        explicit stream_socket(socket&& socket)
        {
            _socket = std::move(socket);
        }

        explicit stream_socket(
            const std::string &ip,
            const uint16_t port)
        {
            _socket = stream_socket_core::new_client_socket(ip, port);
        }

        stream_socket()     = default;
        ~stream_socket()    = default;

        stream_socket(const stream_socket&)             = delete;
        stream_socket& operator=(const stream_socket&)  = delete;

        stream_socket(stream_socket&&)         noexcept = default;
        stream_socket& operator=(stream_socket&&)       = default;

        BANKER_NODISCARD bool is_valid() const
        {
            return _socket.is_valid();
        }

        BANKER_NODISCARD socket& raw_socket()
        {
            return _socket;
        }

        BANKER_NODISCARD std::vector<uint8_t>& receive()
        {
            return _receive_state.receive_buffer;
        }

        void enqueue(const std::vector<uint8_t>& data)
        {
            stream_socket_core::enqueue(_send_state,data);
        }

        void enqueue(std::vector<uint8_t>&& data)
        {
            stream_socket_core::enqueue(_send_state, std::move(data));
        }

        size_t tick(
            const bool readable = true,
            const bool writable = true,
            tcp::request_result* result = nullptr)
        {
            tcp::request_result local_result;
            size_t new_data = 0;
            if ( readable )
            {
                new_data =
                    stream_socket_core::receive(_socket,_receive_state, &local_result);
                if (local_result != tcp::request_result::ok)
                    goto stream_socket_tick_return;
            }
            if ( writable )
            {
                stream_socket_core::flush_out_buffer(_socket,_send_state, &local_result);
                if (local_result != tcp::request_result::ok)
                    goto stream_socket_tick_return;
            }

        stream_socket_tick_return:
            BANKER_SAFE(result) = local_result;
            return new_data;
        }

    private:
        socket _socket;
        stream_socket_core::receive_state   _receive_state;
        stream_socket_core::send_state      _send_state;

    public:
        class acceptor
        {
        public:
            acceptor(
                const std::string &host,
                const uint16_t port)
            {
                _socket = stream_socket_core::new_server_socket(host,port);
            }

            acceptor()  = delete;
            ~acceptor() = default;

            acceptor(const acceptor&)               = delete;
            acceptor& operator=(const acceptor&)    = delete;

            acceptor(acceptor&&) noexcept               = default;
            acceptor& operator=(acceptor&&) noexcept    = default;

            BANKER_NODISCARD bool is_valid() const
            {
                return _socket.is_valid();
            }

            BANKER_NODISCARD bool touch(const int timeout_ms = 0) const
            {
                return _socket.is_readable(timeout_ms);
            }

            BANKER_NODISCARD stream_socket accept()
            {
                return stream_socket{_socket.accept()};
            }

            BANKER_NODISCARD socket& raw_socket()
            {
                return _socket;
            }

        private:
            socket _socket;
        };
    };
}

#endif //BANKER_STREAM_SOCKET_HPP
