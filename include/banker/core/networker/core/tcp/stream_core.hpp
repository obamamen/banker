/* ================================== *\
 @file     stream_core.hpp
 @project  banker
 @author   moosm
 @date     11/28/2025
*\ ================================== */

#ifndef BANKER_STREAM_CORE_HPP
#define BANKER_STREAM_CORE_HPP

#include <deque>

#include "out_buffer.hpp"
#include "banker/core/networker/core/packet_streaming/packet_stream_handler.hpp"
#include "banker/core/networker/core/socket/socket.hpp"

namespace banker::networker
{
    class stream_core
    {
    public:
        struct send_data
        {
            bool sent_current_request{false};
            size_t total_send_requests{0};
        };

        bool tick_receive(socket& socket)
        {
            return tcp::stream_socket(
                socket,
                _receive_buffer);
        }

        std::vector<uint8_t>& receive()
        {
            return _receive_buffer;
        }

        send_data send_data(socket& socket, uint8_t* data, const size_t size)
        {
            const auto t = flush_send_buffer(socket);
            auto s = _send(socket, socket::iovec_c{data,size});
            return {s,t+s};
        }

        size_t flush_send_buffer(socket& socket)
        {
            size_t total_send = 0;

            while ( !_send_buffer.empty() )
            {
                auto& buffer = _send_buffer.front();
                const auto size = static_cast<int>(buffer.size());

                const int sent = socket.send(
                    buffer.data(),
                    buffer.size());

                if (sent == size) // buffer could send fully
                {
                    total_send++;
                    _send_buffer.pop_front();
                    continue;
                }

                if (sent < 0) break; // error / or cant write.

                if (sent < size) // partial send, erase what did send
                {
                    buffer.consume( static_cast<size_t>(sent) );
                    break;
                }
            }

            return total_send;
        }

    private:
        std::vector<uint8_t>        _receive_buffer{};
        std::deque<tcp::out_buffer> _send_buffer{};

        bool _send(socket& socket, const socket::iovec_c buffer)
        {
            const int sent = socket.send(
                buffer.data,
                1);

            if (sent < 0) return false;
            BANKER_LIKELY if (sent == static_cast<int>(buffer.len)) return true;

            _send_buffer.push_back(tcp::out_buffer(
                static_cast<const uint8_t*>(buffer.data) + sent,
                buffer.len - static_cast<size_t>(sent)));

            return false;
        }

        bool _send(socket& socket, const std::span< socket::iovec_c > buffers)
        {
            if (buffers.empty()) return false;

            int total_request_size = 0;
            for (const auto& buffer : buffers)
                { total_request_size += buffer.len; }

            const int sent = socket.sendv(
                buffers.data(),
                buffers.size());

            if (sent < 0) return false;
            BANKER_LIKELY if (sent == total_request_size) return true;

            size_t remaining = static_cast<size_t>(sent);

            for (const auto& buffer : buffers)
            {
                if (remaining >= buffer.len)
                {
                    remaining -= buffer.len;
                    continue;
                }

                if (remaining > 0)
                {
                    _send_buffer.push_back(tcp::out_buffer(
                        static_cast<const uint8_t*>(buffer.data) + remaining,
                        buffer.len - remaining));
                    remaining = 0;
                }
                else
                {
                    _send_buffer.push_back(tcp::out_buffer(
                        static_cast<const uint8_t*>(buffer.data),
                        buffer.len));
                }
            }

            return false;
        }
    };
}

#endif //BANKER_STREAM_CORE_HPP