/* ================================== *\
 @file     variant_buffer.hpp
 @project  banker
 @author   moosm
 @date     12/11/2025
*\ ================================== */

#ifndef BANKER_VARIANT_BUFFER_HPP
#define BANKER_VARIANT_BUFFER_HPP

#include <cstdint>
#include <memory>
#include <vector>

namespace banker
{
    class variant_buffer
    {
    private:
        enum class variant : uint8_t
        {
            vector,
            shared
        };

    public:
        variant_buffer() = default;
        ~variant_buffer()
        {
            switch (_variant)
            {
                case variant::vector:
                    _buffer.~vector();
                    break;

                case variant::shared:
                    _shared_buffer.~shared_ptr();
                    break;
            }
        }

        variant_buffer(const variant_buffer&) = delete;
        variant_buffer& operator=(const variant_buffer&) = delete;

        variant_buffer(variant_buffer&&) noexcept = default;
        variant_buffer& operator=(variant_buffer&&) noexcept = default;

    private:
        variant _variant{variant::vector};
        union
        {
            std::vector<uint8_t>                    _buffer{};
            std::shared_ptr<std::vector<uint8_t>>   _shared_buffer;
        };
    };
}

#endif //BANKER_VARIANT_BUFFER_HPP
