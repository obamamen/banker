//
// Created by moosm on 11/4/2025.
//

#ifndef BANKER_FORMAT_BYTES_HPP
#define BANKER_FORMAT_BYTES_HPP

#include <iomanip>
#include <sstream>
#include <string>
#include <bitset>
#include <stdint.h>

namespace banker::format_bytes
{
    inline std::string to_hex_bytes(
        const unsigned char* bytes,
        const size_t len,
        const std::string& separator = "",
        const size_t width = 1)
    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (size_t i = 0; i < len; ++i)
        {
            const bool do_sep = (i != len - 1) && ((i + 1) % width == 0);
            oss << std::setw(2) << static_cast<uint16_t>(bytes[i]);
            if (do_sep) oss << separator;
        }

        return oss.str();
    }


    template<typename T>
    inline std::string to_hex(
        const T& value,
        const std::string& separator = "",
        const size_t width = 1)
    {
        return to_hex_bytes(reinterpret_cast<const unsigned char*>(&value),
                            sizeof(T), separator, width);
    }

    template<typename T, size_t N>
    inline std::string to_hex(const T (&arr)[N],
                              const std::string& separator = "",
                              size_t width = 1)
    {
        return to_hex_bytes(reinterpret_cast<const unsigned char*>(arr),
                            sizeof(T) * N, separator, width);
    }

    inline std::string to_hex(
        const uint8_t* data,
        const size_t len,
        const std::string& separator = "",
        const size_t width = 1)
    {
        return to_hex_bytes(data, len, separator, width);
    }

    inline std::string to_hex(
        const std::vector<uint8_t>& data,
        const std::string& separator = "",
        const size_t width = 1)
    {
        return to_hex_bytes(data.data(), data.size(), separator, width);
    }

    class details
    {
    public:
        static std::string to_base64_impl(
            const unsigned char* bytes,
            const size_t len,
            const char* alphabet,
            const bool use_padding)
        {
            std::string result;
            result.reserve(((len + 2) / 3) * 4);

            for (size_t i = 0; i < len; i += 3)
            {
                uint32_t n = static_cast<uint32_t>(bytes[i]) << 16;
                if (i + 1 < len) n |= static_cast<uint32_t>(bytes[i + 1]) << 8;
                if (i + 2 < len) n |= static_cast<uint32_t>(bytes[i + 2]);

                result += alphabet[(n >> 18) & 0x3F];
                result += alphabet[(n >> 12) & 0x3F];
                if (i + 1 < len) result += alphabet[(n >> 6) & 0x3F];
                else if (use_padding) result += '=';
                if (i + 2 < len) result += alphabet[n & 0x3F];
                else if (use_padding) result += '=';
            }

            return result;
        }

        static std::string to_base32_impl(const unsigned char* bytes, const size_t len)
        {
            static constexpr auto b32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
            std::string result;
            result.reserve(((len + 4) / 5) * 8);

            for (size_t i = 0; i < len; i += 5)
            {
                uint64_t n = 0;
                size_t available = (len - i < 5) ? (len - i) : 5;

                for (size_t j = 0; j < available; ++j)
                    n = (n << 8) | bytes[i + j];

                n <<= (5 - available) * 8;

                for (int j = 35; j >= 0; j -= 5)
                {
                    if (j >= static_cast<int>((5 - available) * 8))
                        result += b32_chars[(n >> j) & 0x1F];
                    else
                        result += '=';
                }
            }

            return result;
        }
    };

    template<typename T>
    std::string to_b64(const T& value)
    {
        static constexpr auto b64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        return details::to_base64_impl(reinterpret_cast<const unsigned char*>(&value),
                                       sizeof(T), b64_chars, true);
    }

    inline std::string to_b64(const uint8_t* data, const size_t len)
    {
        static constexpr auto b64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        return details::to_base64_impl(data, len, b64_chars, true);
    }

    template<typename T>
    std::string to_binary(
        const T& value,
        const std::string& separator = " ",
        const size_t width = 1)
    {
        const auto bytes = reinterpret_cast<const unsigned char*>(&value);
        std::string result;
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            result += std::bitset<8>(bytes[i]).to_string();
            const bool do_sep = (i != sizeof(T) - 1) && ((i + 1) % width == 0);
            if (do_sep) result += separator;
        }
        return result;
    }

    inline std::string to_binary(
        const uint8_t* data,
        const size_t len,
        const std::string& separator = " ",
        const size_t width = 1)
    {
        std::string result;
        for (size_t i = 0; i < len; ++i)
        {
            result += std::bitset<8>(data[i]).to_string();
            const bool do_sep = (i != len - 1) && ((i + 1) % width == 0);
            if (do_sep) result += separator;
        }
        return result;
    }
}

#endif //BANKER_FORMAT_BYTES_HPP