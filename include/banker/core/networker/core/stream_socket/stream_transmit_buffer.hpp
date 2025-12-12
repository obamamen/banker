/* ================================== *\
 @file     stream_transmit_buffer.hpp
 @project  banker
 @author   moosm
 @date     12/11/2025
*\ ================================== */

#ifndef BANKER_STREAM_TRANSMIT_BUFFER_HPP
#define BANKER_STREAM_TRANSMIT_BUFFER_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "banker/shared/compat.hpp"

namespace banker::networker
{
    class stream_transmit_buffer
    {
    public:
        stream_transmit_buffer()    = default;
        ~stream_transmit_buffer()   = default;

        stream_transmit_buffer(const stream_transmit_buffer&) = delete;
        stream_transmit_buffer& operator=(const stream_transmit_buffer&) = delete;

        stream_transmit_buffer(stream_transmit_buffer&& other) noexcept = default;
        stream_transmit_buffer& operator=(stream_transmit_buffer&& other) noexcept = default;

        explicit stream_transmit_buffer(const std::vector<uint8_t>& buffer)
        {
            _buffer = buffer;
        }

        explicit stream_transmit_buffer(std::vector<uint8_t>&& buffer) noexcept
        {
            _buffer = std::move(buffer);
        }

        explicit stream_transmit_buffer(const uint8_t* data, const size_t size) noexcept
        {
            _buffer = std::vector(data, data + size);
        }

        BANKER_NODISCARD uint8_t* data(const size_t offset) const
        {
            return const_cast<uint8_t *>(_buffer.data() + offset);
        }

        BANKER_NODISCARD socket::iovec_c to_iovec(const size_t offset) const
        {
            return socket::iovec_c{
                data(offset),
                size(offset)};
        }

        BANKER_NODISCARD size_t size(const size_t offset) const
        {
            return _buffer.size() - offset;
        }

        BANKER_NODISCARD size_t consume(
            const size_t bytes,
            size_t& offset) const
        {
            const auto available = size(offset);

            if (bytes <= available)
            {
                offset += bytes;
                return 0;
            }
            else
            {
                offset += available;
                return bytes - available;
            }
        }

    private:
        std::vector<uint8_t> _buffer{};
    };
}

#endif //BANKER_STREAM_TRANSMIT_BUFFER_HPP
