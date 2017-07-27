// cm_color.h
//

#pragma once

#include "cm_vector.h"

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
class color3
{
public:
    float   r;
    float   g;
    float   b;

// constructors

    color3() = default;
    constexpr color3(float R, float G, float B) : r(R), g(G), b(B) {}
    constexpr explicit color3(vec3 const& V) : r(V.x), g(V.y), b(V.z) {}

    color3& operator=(const color3 &C) {r=C.r; g=C.g; b=C.b; return *this; }
    bool operator==(const color3 &C) const {return r == C.r && g == C.g && b == C.b; }
    bool operator!=(const color3 &C) const {return r != C.r || g != C.g || b != C.b; }
    float operator[](std::size_t idx) const { return (&r)[idx]; }
    float& operator[](std::size_t idx) { return (&r)[idx]; }
    operator float*() { return &r; }
    operator float const*() const { return &r; }

// algebraic vector operations

    color3 operator-() const { return color3(-r, -g, -b); }
    color3 operator+(color3 const& C) const { return color3(r+C.r, g+C.g, b+C.b); }
    color3 operator-(color3 const& C) const { return color3(r-C.r, g-C.g, b-C.b); }
    color3 operator*(color3 const& C) const { return color3(r*C.r, g*C.g, b*C.b); }
    color3 operator/(color3 const& C) const { return color3(r/C.r, g/C.g, b/C.b); }
    color3 operator*(float S) const { return color3(r*S, g*S, b*S); }
    color3 operator/(float S) const { return color3(r/S, g/S, b/S); }

// algebraic vector assignment operations

    color3& operator+=(color3 const& C) { r+=C.r; g+=C.g; b+=C.b; return *this; }
    color3& operator-=(color3 const& C) { r-=C.r; g-=C.g; b-=C.b; return *this; }
    color3& operator*=(color3 const& C) { r*=C.r; g*=C.g; b*=C.b; return *this; }
    color3& operator/=(color3 const& C) { r/=C.r; g/=C.g; b/=C.b; return *this; }
    color3& operator*=(float S) { r*=S; g*=S; b*=S; return *this; }
    color3& operator/=(float S) { r/=S; g/=S; b/=S; return *this; }

// utility functions

    void clear() { r=0.0f; g=0.0f; b=0.0f; }
};

//------------------------------------------------------------------------------
class color4
{
public:
    float   r;
    float   g;
    float   b;
    float   a;

// constructors

    color4() = default;
    constexpr color4(float X, float Y, float Z, float W = 1) : r(X), g(Y), b(Z), a(W) {}
    constexpr explicit color4(color3 const& C, float A = 1) : r(C.r), g(C.g), b(C.b), a(A) {}
    constexpr explicit color4(vec4 const& V) : r(V.x), g(V.y), b(V.z), a(V.w) {}

    color4& operator=(const color4 &C) { r=C.r; g=C.g; b=C.b; a=C.a; return *this; }
    bool operator==(const color4 &C) const { return r==C.r && g==C.g && b==C.b && a==C.a; }
    bool operator!=(const color4 &C) const { return r!=C.r || g!=C.g || b!=C.b || a!=C.a; }
    float operator[](std::size_t idx) const { return (&r)[idx]; }
    float& operator[](std::size_t idx) { return (&r)[idx]; }
    operator float*() { return &r; }
    operator float const*() const { return &r; }

// algebraic vector operations

    color4 operator-() const { return color4(-r, -g, -b, -a); }
    color4 operator+(color4 const& C) const { return color4(r+C.r, g+C.g, b+C.b, a+C.a); }
    color4 operator-(color4 const& C) const { return color4(r-C.r, g-C.g, b-C.b, a-C.a); }
    color4 operator*(color4 const& C) const { return color4(r*C.r, g*C.g, b*C.b, a*C.a); }
    color4 operator/(color4 const& C) const { return color4(r/C.r, g/C.g, b/C.b, a/C.a); }
    color4 operator*(float S) const { return color4(r*S, g*S, b*S, a*S); }
    color4 operator/(float S) const { return color4(r/S, g/S, b/S, a/S); }

// algebraic vector assignment operations

    color4& operator+=(color4 const& C) { r+=C.r; g+=C.g; b+=C.b; a+=C.a; return *this; }
    color4& operator-=(color4 const& C) { r-=C.r; g-=C.g; b-=C.b; a-=C.a; return *this; }
    color4& operator*=(color4 const& C) { r*=C.r; g*=C.g; b*=C.b; a*=C.a; return *this; }
    color4& operator/=(color4 const& C) { r/=C.r; g/=C.g; b/=C.b; a/=C.a; return *this; }
    color4& operator*=(float S) { r*=S; g*=S; b*=S; a*=S; return *this; }
    color4& operator/=(float S) { r/=S; g/=S; b/=S; a/=S; return *this; }

// utility functions

    void clear() { r=0.0f; g=0.0f; b=0.0f; a=0.0f; }
    color3 to_color3() const { return color3(r, g, b); }
};
