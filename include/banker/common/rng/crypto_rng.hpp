//
// Created by moosm on 11/4/2025.
//

#ifndef BANKER_CRYPTO_RNG_HPP
#define BANKER_CRYPTO_RNG_HPP

#include <cstdint>

#ifdef _WIN32
    #include <windows.h>
    #include <bcrypt.h>
#else
    #include <fstream>
#endif

namespace banker
{
    class crypto_rng
    {
    public:
        template<typename T>
        static bool get(T& buffer)
        {
            return get_bytes(&buffer, sizeof(T));
        }

        static bool get_bytes(void* buffer, const size_t size)
        {
            if (!buffer || size == 0) return false;
#ifdef _WIN32
            const NTSTATUS status = BCryptGenRandom(
             nullptr,
             static_cast<PUCHAR>(buffer),
             size,
             BCRYPT_USE_SYSTEM_PREFERRED_RNG
            );

            if (status != 0)
            {
                return false;
            }
#else
            std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
            if (!urandom)
            {
                return false;
            }

            urandom.read(static_cast<char*>(buffer), size);
            if (!urandom || urandom.gcount() != static_cast<std::streamsize>(size))
            {
                return false;
            }
#endif

            return true;
        }
    };
}

#endif //BANKER_CRYPTO_RNG_HPP