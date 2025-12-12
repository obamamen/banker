/* ================================== *\
 @file     program_macros.hpp
 @project  banker
 @author   moosm
 @date     11/12/2025
*\ ================================== */

#ifndef BANKER_PROGRAM_MACROS_HPP
#define BANKER_PROGRAM_MACROS_HPP

#define BANKER_SAFE(out) if ( (out != nullptr) ) (*out)

#ifdef NDEBUG
#define BANKER_DEBUG_DO(...) do {} while(0)
#else
#define BANKER_DEBUG_DO(...) do { __VA_ARGS__; } while(0)
#endif

#define BANKER_SHOULD(thing) do { if (!(thing)) throw std::runtime_error(#thing);  } while(0)

#define BANKER_TERMINATE(msg)                       \
do                                                  \
{                                                   \
    std::cerr << "FATAL ERROR: " << (msg)           \
        << "\n  at " << __FILE__                    \
        << ":" << __LINE__                          \
        << " (" << __func__ << ")" << std::endl;    \
    std::cerr.flush();                              \
    std::terminate();                               \
} while(0)

#endif //BANKER_PROGRAM_MACROS_HPP