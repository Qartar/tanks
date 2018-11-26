// cm_shared.h
//

#pragma once

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>

#define APP_CLASSNAME       "Tanks!"

#define PORT_SERVER     28101
#define PORT_CLIENT     28110

//------------------------------------------------------------------------------
using byte = std::uint8_t;
using word = std::uint16_t;

#include "cm_vector.h"
#include "cm_matrix.h"
#include "cm_color.h"
#include "cm_error.h"

#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data

#define MAX_STRING      1024
#define LONG_STRING     256
#define SHORT_STRING    32

//------------------------------------------------------------------------------
#ifndef M_LN2
#define M_LN2   0.693147180559945309417
#endif

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#ifndef M_SQRT2
#define M_SQRT2    1.41421356237309504880
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401
#endif

//------------------------------------------------------------------------------
template<typename T> constexpr T deg2rad(T value) { return value * T(M_PI / 180.0); }
template<typename T> constexpr T rad2deg(T value) { return value * T(180.0 / M_PI); }

__forceinline float frand() { return ((float)((rand()&32767)*(1.0f/32767.0f))); }
__forceinline float crand() { return ((float)((rand()&32767)*(2.0f/32767.0f)-1.0f)); }

template<typename T> constexpr T square(T value) { return value * value; }

template<typename T> constexpr T clamp(T value, T min, T max) { return (value < min) ? min : (value > max) ? max : value; }

template<typename T> constexpr T min(T a, T b) { return a < b ? a : b; }
template<typename T> constexpr T max(T a, T b) { return a > b ? a : b; }

//------------------------------------------------------------------------------
template<typename T, std::size_t size> constexpr std::size_t countof(T const (&)[size])
{
    return size;
}

//------------------------------------------------------------------------------
template<typename T> constexpr std::size_t countof()
{
    return countof(T{});
}

//------------------------------------------------------------------------------
template<typename T, typename Y> constexpr T narrow_cast(Y value)
{
    assert(static_cast<Y>(static_cast<T>(value)) == value);
    return static_cast<T>(value);
}

//------------------------------------------------------------------------------
char const* va (char const *fmt, ...);

//------------------------------------------------------------------------------
namespace detail {

class log
{
public:
    enum class level {
        message,
        warning,
        error,
    };

    //! set the logging singleton
    static void set(log* logger);

    static void message(char const* fmt, ...);
    static void warning(char const* fmt, ...);
    static void error(char const* fmt, ...);

protected:
    virtual void print(level level, char const* message) = 0;

private:
    static log* _singleton;
};

} // namespace detail

using namespace detail; // workaround for namespace collision with <cmath>
