//
// Created by moosm on 11/4/2025.
//

#ifndef BANKER_FORMAT_BYTES_HPP
#define BANKER_FORMAT_BYTES_HPP

#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <bitset>
#include <cstdint>

namespace banker::format_bytes
{
    constexpr std::array<std::array<char, 2>, 256> make_hex_table()
    {
        std::array<std::array<char, 2>, 256> table{};
        for (std::size_t i = 0; i < 256; ++i)
        {
            constexpr char hex_chars[] = "0123456789abcdef";
            table[i][0] = hex_chars[i >> 4];
            table[i][1] = hex_chars[i & 0xF];
        }
        return table;
    }

    inline constexpr auto hex_table = make_hex_table();

    inline std::string to_hex_bytes(
        const unsigned char* bytes,
        const size_t len,
        const std::string& separator = "",
        const size_t width = 1)
    {
        std::ostringstream oss_local;
        std::ostream& out = oss_local;

        out << std::hex << std::setfill('0');

        for (size_t i = 0; i < len; ++i)
        {
            const bool do_sep = (i != len - 1) && ((i + 1) % width == 0);
            out << std::setw(2) << static_cast<uint16_t>(bytes[i]);
            if (do_sep) out << separator;
        }
        return oss_local.str();
    }

    inline void to_hex_bytes_stream(
    const uint8_t* bytes,
    const size_t len,
    std::ostream& out,
    const std::string& separator = "",
    size_t width = 0)
    {
        if (width == 0 || width > len) width = len;

        constexpr size_t MAX_CHUNK_BYTES = 64 * 1024;

        if (len <= MAX_CHUNK_BYTES && width == len)
        {
            for (size_t i = 0; i < len; ++i)
                out.write(hex_table[bytes[i]].data(), 2);
            return;
        }

        size_t pos = 0;
        while (pos < len)
        {
            size_t chunk_bytes = (len - pos > MAX_CHUNK_BYTES) ? MAX_CHUNK_BYTES : (len - pos);
            size_t chunk_width = (chunk_bytes < width) ? chunk_bytes : width;

            std::vector<char> buf(chunk_width * 2);

            for (size_t i = 0; i < chunk_width; ++i)
            {
                const auto& hex = hex_table[bytes[pos + i]];
                buf[i * 2 + 0] = hex[0];
                buf[i * 2 + 1] = hex[1];
            }

            out.write(buf.data(), chunk_width * 2);

            if (!separator.empty() && pos + chunk_width < len)
                out.write(separator.data(), separator.size());

            pos += chunk_width;
        }
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