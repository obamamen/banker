/* ================================== *\
 @file     compat.hpp
 @project  banker
 @author   moosm
 @date     11/12/2025
*\ ================================== */

#ifndef BANKER_COMPAT_HPP
#define BANKER_COMPAT_HPP

#ifndef BANKER_CPP_VERSION
#  define BANKER_CPP_VERSION __cplusplus
#endif

#define BANKER_CPP98_PLUS (__cplusplus >= 199711L)
#define BANKER_CPP11_PLUS (__cplusplus >= 201103L)
#define BANKER_CPP14_PLUS (__cplusplus >= 201402L)
#define BANKER_CPP17_PLUS (__cplusplus >= 201703L)
#define BANKER_CPP20_PLUS (__cplusplus >= 202002L)
#define BANKER_CPP23_PLUS (__cplusplus >= 202302L)

#if defined(_MSC_VER)
#  define BANKER_COMPILER_MSVC 1
#elif defined(__clang__)
#  define BANKER_COMPILER_CLANG 1
#elif defined(__GNUC__)
#  define BANKER_COMPILER_GCC 1
#endif

#if defined(_WIN32) || defined(_WIN64)
#  define BANKER_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#  define BANKER_PLATFORM_APPLE 1
#elif defined(__linux__)
#  define BANKER_PLATFORM_LINUX 1
#elif defined(__unix__) || defined(__unix)
#  define BANKER_PLATFORM_UNIX 1
#endif

#if defined(__x86_64__) || defined(_M_X64)
#  define BANKER_ARCH_X64 1
#elif defined(__i386__) || defined(_M_IX86)
#  define BANKER_ARCH_X86 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#  define BANKER_ARCH_ARM64 1
#elif defined(__arm__) || defined(_M_ARM)
#  define BANKER_ARCH_ARM 1
#endif

#if !defined(NDEBUG) || defined(_DEBUG)
#  define BANKER_DEBUG 1
#else
#  define BANKER_RELEASE 1
#endif

#if BANKER_CPP20_PLUS
#  define BANKER_UNLIKELY [[unlikely]]
#  define BANKER_LIKELY   [[likely]]
#else
#  define BANKER_UNLIKELY
#  define BANKER_LIKELY
#endif

#if BANKER_CPP17_PLUS
#  define BANKER_NODISCARD [[nodiscard]]
#  define BANKER_MAYBE_UNUSED [[maybe_unused]]
#  define BANKER_FALLTHROUGH [[fallthrough]]
#else
#  define BANKER_NODISCARD
#  define BANKER_MAYBE_UNUSED
#  define BANKER_FALLTHROUGH
#endif

#if BANKER_CPP11_PLUS
#  define BANKER_NORETURN [[noreturn]]
#elif defined(BANKER_COMPILER_MSVC)
#  define BANKER_NORETURN __declspec(noreturn)
#elif defined(BANKER_COMPILER_GCC) || defined(BANKER_COMPILER_CLANG)
#  define BANKER_NORETURN __attribute__((noreturn))
#else
#  define BANKER_NORETURN
#endif

#if BANKER_CPP11_PLUS
#  define BANKER_CONSTEXPR constexpr
#else
#  define BANKER_CONSTEXPR inline
#endif

#if BANKER_CPP14_PLUS
#  define BANKER_CONSTEXPR14 constexpr
#else
#  define BANKER_CONSTEXPR14 inline
#endif

#if BANKER_CPP17_PLUS
#  define BANKER_CONSTEXPR17 constexpr
#else
#  define BANKER_CONSTEXPR17 inline
#endif

#if BANKER_CPP20_PLUS
#  define BANKER_CONSTEXPR20 constexpr
#  define BANKER_CONSTEVAL constexpr
#  define BANKER_CONSTINIT
#else
#  define BANKER_CONSTEXPR20 inline
#  define BANKER_CONSTEVAL inline
#  define BANKER_CONSTINIT
#endif

#if BANKER_CPP11_PLUS
#  define BANKER_NOEXCEPT noexcept
#  define BANKER_NOEXCEPT_IF(x) noexcept(x)
#else
#  define BANKER_NOEXCEPT throw()
#  define BANKER_NOEXCEPT_IF(x)
#endif

#if BANKER_CPP11_PLUS
#  define BANKER_OVERRIDE override
#  define BANKER_FINAL final
#else
#  define BANKER_OVERRIDE
#  define BANKER_FINAL
#endif

#if BANKER_CPP11_PLUS
#  define BANKER_NULLPTR nullptr
#else
#  define BANKER_NULLPTR NULL
#endif

#if BANKER_CPP11_PLUS
#  define BANKER_DELETE = delete
#  define BANKER_DEFAULT = default
#else
#  define BANKER_DELETE
#  define BANKER_DEFAULT {}
#endif

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define BANKER_BIG_ENDIAN 1
#elif defined(_WIN32)
#  define BANKER_LITTLE_ENDIAN 1
#else
#  define BANKER_LITTLE_ENDIAN 1
#endif

#if defined(BANKER_COMPILER_MSVC)
#  define BANKER_FORCEINLINE __forceinline
#elif defined(BANKER_COMPILER_GCC) || defined(BANKER_COMPILER_CLANG)
#  define BANKER_FORCEINLINE inline __attribute__((always_inline))
#else
#  define BANKER_FORCEINLINE inline
#endif

#if defined(BANKER_COMPILER_MSVC)
#  define BANKER_NOINLINE __declspec(noinline)
#elif defined(BANKER_COMPILER_GCC) || defined(BANKER_COMPILER_CLANG)
#  define BANKER_NOINLINE __attribute__((noinline))
#else
#  define BANKER_NOINLINE
#endif

#if defined(BANKER_PLATFORM_WINDOWS)
#  if defined(BANKER_BUILD_SHARED)
#    define BANKER_API __declspec(dllexport)
#  elif defined(BANKER_USE_SHARED)
#    define BANKER_API __declspec(dllimport)
#  else
#    define BANKER_API
#  endif
#else
#  if defined(BANKER_COMPILER_GCC) || defined(BANKER_COMPILER_CLANG)
#    define BANKER_API __attribute__((visibility("default")))
#  else
#    define BANKER_API
#  endif
#endif

#if BANKER_CPP14_PLUS
#  define BANKER_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(BANKER_COMPILER_MSVC)
#  define BANKER_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(BANKER_COMPILER_GCC) || defined(BANKER_COMPILER_CLANG)
#  define BANKER_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#  define BANKER_DEPRECATED(msg)
#endif

#if defined(BANKER_COMPILER_MSVC)
#  define BANKER_UNREACHABLE() __assume(0)
#elif defined(BANKER_COMPILER_GCC) || defined(BANKER_COMPILER_CLANG)
#  define BANKER_UNREACHABLE() __builtin_unreachable()
#else
#  define BANKER_UNREACHABLE() do {} while(0)
#endif

#ifdef BANKER_DEBUG
#  include <cassert>
#  define BANKER_ASSERT(cond) assert(cond)
#  define BANKER_ASSERT_MSG(cond, msg) assert((cond) && (msg))
#else
#  define BANKER_ASSERT(cond) ((void)0)
#  define BANKER_ASSERT_MSG(cond, msg) ((void)0)
#endif

#endif //BANKER_COMPAT_HPP