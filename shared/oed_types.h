/*
===========================================================

Name    :   oed_types.h

Purpose :   data type definitions

Modified:   11/03/2006

===========================================================
*/

#pragma once

#include <cmath>   // sqrt
#include <cstdint>
#include <algorithm>

/*
===========================================================

CLASS DEFINITIONS

===========================================================
*/

class vec2;
class vec3;
class vec4;
class mat3;
class mat4;

////////////////////////////////////////////////////////////////////////////////
// vector types

//------------------------------------------------------------------------------
class vec2
{
public:
    float   x;
    float   y;

// constructors

    vec2() = default;
    constexpr vec2(float X, float Y) : x(X), y(Y) {}
    explicit constexpr vec2(float S) : x(S), y(S) {}

    vec2& operator=(const vec2 &V) {x=V.x; y=V.y; return *this;}
    bool operator==(const vec2 &V) const { return x == V.x && y == V.y; }
    bool operator!=(const vec2 &V) const { return x != V.x || y != V.y; }
    float operator[](std::size_t idx) const { return (&x)[idx]; }
    float& operator[](std::size_t idx) { return (&x)[idx]; }
    operator float*() { return &x; }
    operator float const*() const { return &x; }

// algebraic vector operations

    vec2 operator-() const {return vec2(-x, -y);}
    vec2 operator+(vec2 const& V) const { return vec2(x+V.x, y+V.y); }
    vec2 operator-(vec2 const& V) const { return vec2(x-V.x, y-V.y); }
    vec2 operator*(vec2 const& V) const { return vec2(x*V.x, y*V.y); }
    vec2 operator/(vec2 const& V) const { return vec2(x/V.x, y/V.y); }
    vec2 operator*(float S) const { return vec2(x*S, y*S); }
    vec2 operator/(float S) const { return vec2(x/S, y/S); }

// algebraic vector assignment operations

    vec2& operator+=(vec2 const& V) { x+=V.x; y+=V.y; return *this; }
    vec2& operator-=(vec2 const& V) { x-=V.x; y-=V.y; return *this; }
    vec2& operator*=(vec2 const& V) { x*=V.x; y*=V.y; return *this; }
    vec2& operator/=(vec2 const& V) { x/=V.x; y/=V.y; return *this; }
    vec2& operator*=(float S) { x*=S; y*=S; return *this; }
    vec2& operator/=(float S) { x/=S; y/=S; return *this; }

// utility functions

    float length() const { return std::sqrt(length_sqr()); }
    float length_sqr() const { return x*x + y*y; }
    vec2& normalize() { *this /= length(); return *this; }
    float normalize_length() { float len = length(); *this /= len; return len; }
    void clear() { x=0.0f; y=0.0f; }

    float dot(const vec2 &V) const { return x*V.x + y*V.y; }
    vec2 cross(float V) const { return vec2(y*V, -x*V); }
};

//------------------------------------------------------------------------------
class vec3
{
public:
    float   x;
    float   y;
    float   z;

// constructors

    vec3() = default;
    constexpr vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    constexpr explicit vec3(float S) : x(S), y(S), z(S) {}
    constexpr explicit vec3(vec2 const& V, float Z = 0) :x(V.x), y(V.y), z(Z) {}

    vec3& operator=(const vec3 &V) {x=V.x; y=V.y; z=V.z; return *this; }
    bool operator==(const vec3 &V) const {return x == V.x && y == V.y && z == V.z; }
    bool operator!=(const vec3 &V) const {return x != V.x || y != V.y || z != V.z; }
    float operator[](std::size_t idx) const { return (&x)[idx]; }
    float& operator[](std::size_t idx) { return (&x)[idx]; }
    operator float*() { return &x; }
    operator float const*() const { return &x; }

// algebraic vector operations

    vec3 operator-() const { return vec3(-x, -y, -z); }
    vec3 operator+(vec3 const& V) const { return vec3(x+V.x, y+V.y, z+V.z); }
    vec3 operator-(vec3 const& V) const { return vec3(x-V.x, y-V.y, z-V.z); }
    vec3 operator*(vec3 const& V) const { return vec3(x*V.x, y*V.y, z*V.z); }
    vec3 operator/(vec3 const& V) const { return vec3(x/V.x, y/V.y, z/V.z); }
    vec3 operator*(float S) const { return vec3(x*S, y*S, z*S); }
    vec3 operator/(float S) const { return vec3(x/S, y/S, z/S); }

// algebraic vector assignment operations

    vec3& operator+=(vec3 const& V) { x+=V.x; y+=V.y; z+=V.z; return *this; }
    vec3& operator-=(vec3 const& V) { x-=V.x; y-=V.y; z-=V.z; return *this; }
    vec3& operator*=(vec3 const& V) { x*=V.x; y*=V.y; z*=V.z; return *this; }
    vec3& operator/=(vec3 const& V) { x/=V.x; y/=V.y; z/=V.z; return *this; }
    vec3& operator*=(float S) { x*=S; y*=S; z*=S; return *this; }
    vec3& operator/=(float S) { x/=S; y/=S; z/=S; return *this; }

// utility functions

    float length() const {return std::sqrt(length_sqr()); }
    float length_sqr() const {return x*x + y*y + z*z; }
    vec3& normalize() { *this/=length(); return *this; }
    float normalize_length() { float len = length(); *this /= len; return len; }
    void clear() { x=0.0f; y=0.0f; z=0.0f; }

    float dot(vec3 const& V) const { return x*V.x + y*V.y + z*V.z;}
    vec3 cross(vec3 const& V) const { return vec3( y*V.z - z*V.y, z*V.x - x*V.z, x*V.y - y*V.x ); }

    vec2 to_vec2() const { return vec2(x, y); }
};

//------------------------------------------------------------------------------
class vec4
{
public:
    float   x;
    float   y;
    float   z;
    float   w;

// constructors

    vec4() = default;
    constexpr vec4(float X, float Y, float Z, float W = 1) : x(X), y(Y), z(Z), w(W) {}
    constexpr explicit vec4(float S) : x(S), y(S), z(S), w(S) {}
    constexpr explicit vec4(vec3 const& V, float W = 1) : x(V.x), y(V.y), z(V.z), w(W) {}

    vec4& operator=(const vec4 &V) { x=V.x; y=V.y; z=V.z; w=V.w; return *this; }
    bool operator==(const vec4 &V) const { return x==V.x && y==V.y && z==V.z && w==V.w; }
    bool operator!=(const vec4 &V) const { return x!=V.x || y!=V.y || z!=V.z || w!=V.w; }
    float operator[](std::size_t idx) const { return (&x)[idx]; }
    float& operator[](std::size_t idx) { return (&x)[idx]; }
    operator float*() { return &x; }
    operator float const*() const { return &x; }

// algebraic vector operations

    vec4 operator-() const { return vec4(-x, -y, -z, -w); }
    vec4 operator+(vec4 const& V) const { return vec4(x+V.x, y+V.y, z+V.z, w+V.w); }
    vec4 operator-(vec4 const& V) const { return vec4(x-V.x, y-V.y, z-V.z, w-V.w); }
    vec4 operator*(vec4 const& V) const { return vec4(x*V.x, y*V.y, z*V.z, w*V.w); }
    vec4 operator/(vec4 const& V) const { return vec4(x/V.x, y/V.y, z/V.z, w/V.w); }
    vec4 operator*(float S) const { return vec4(x*S, y*S, z*S, w*S); }
    vec4 operator/(float S) const { return vec4(x/S, y/S, z/S, w/S); }

// algebraic vector assignment operations

    vec4& operator+=(vec4 const& V) { x+=V.x; y+=V.y; z+=V.z; w+=V.w; return *this; }
    vec4& operator-=(vec4 const& V) { x-=V.x; y-=V.y; z-=V.z; w-=V.w; return *this; }
    vec4& operator*=(vec4 const& V) { x*=V.x; y*=V.y; z*=V.z; w*=V.w; return *this; }
    vec4& operator/=(vec4 const& V) { x/=V.x; y/=V.y; z/=V.z; w/=V.w; return *this; }
    vec4& operator*=(float S) { x*=S; y*=S; z*=S; w*=S; return *this; }
    vec4& operator/=(float S) { x/=S; y/=S; z/=S; w/=S; return *this; }

// utility functions

    float length() const { return std::sqrt(length_sqr()); }
    float length_sqr() const { return x*x + y*y + z*z + w*w; }
    vec4& normalize() { *this/=length(); return *this; }
    float normalize_length() { float len = length(); *this /= len; return len; }
    void clear() { x=0.0f; y=0.0f; z=0.0f; w=0.0f; }

    float dot(vec4 const& V) const { return x*V.x + y*V.y + z*V.z + w*V.w; }
    vec4 cross(vec4 const& V) const { return vec4(y*V.z - z*V.y, z*V.x - x*V.z, x*V.y - y*V.x, 0.0f); }

    vec3 to_vec3() const { return vec3(x, y, z); }
};

////////////////////////////////////////////////////////////////////////////////
// color types

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

////////////////////////////////////////////////////////////////////////////////
// integer vector types

//------------------------------------------------------------------------------
class vec2i
{
public:
    int   x;
    int   y;

// constructors

    vec2i() = default;
    constexpr vec2i(int X, int Y) : x(X), y(Y) {}
    explicit constexpr vec2i(int S) : x(S), y(S) {}

    vec2i& operator=(const vec2i &V) {x=V.x; y=V.y; return *this;}
    bool operator==(const vec2i &V) const { return x == V.x && y == V.y; }
    bool operator!=(const vec2i &V) const { return x != V.x || y != V.y; }
    int operator[](std::size_t idx) const { return (&x)[idx]; }
    int& operator[](std::size_t idx) { return (&x)[idx]; }
    operator int*() { return &x; }
    operator int const*() const { return &x; }

// algebraic vector operations

    vec2i operator-() const {return vec2i(-x, -y);}
    vec2i operator+(vec2i const& V) const { return vec2i(x+V.x, y+V.y); }
    vec2i operator-(vec2i const& V) const { return vec2i(x-V.x, y-V.y); }
    vec2i operator*(vec2i const& V) const { return vec2i(x*V.x, y*V.y); }
    vec2i operator/(vec2i const& V) const { return vec2i(x/V.x, y/V.y); }
    vec2i operator*(int S) const { return vec2i(x*S, y*S); }
    vec2i operator/(int S) const { return vec2i(x/S, y/S); }

// algebraic vector assignment operations

    vec2i& operator+=(vec2i const& V) { x+=V.x; y+=V.y; return *this; }
    vec2i& operator-=(vec2i const& V) { x-=V.x; y-=V.y; return *this; }
    vec2i& operator*=(vec2i const& V) { x*=V.x; y*=V.y; return *this; }
    vec2i& operator/=(vec2i const& V) { x/=V.x; y/=V.y; return *this; }
    vec2i& operator*=(int S) { x*=S; y*=S; return *this; }
    vec2i& operator/=(int S) { x/=S; y/=S; return *this; }

// utility functions

    double length() const { return std::sqrt(length_sqr()); }
    int length_sqr() const { return x*x + y*y; }
    void clear() { x=0; y=0; }

    int dot(const vec2i &V) const { return x*V.x + y*V.y; }
    vec2i cross(int V) const { return vec2i(y*V, -x*V); }
    explicit operator vec2() const { return vec2((float)x, (float)y); }
};

////////////////////////////////////////////////////////////////////////////////
// integer rectangle

//------------------------------------------------------------------------------
class rect
{
public:

// constructors

    rect() = default;
    constexpr rect(int X, int Y, int W, int H) : _mins(X, Y), _maxs(W, H) {}
    constexpr rect(vec2i mins, vec2i maxs) : _mins(mins), _maxs(maxs) {}

    rect& operator=(const rect &R) { _mins=R._mins; _maxs=R._maxs; return *this; }
    bool operator==(const rect &R) const { return _mins == R._mins && _maxs == R._maxs; }
    bool operator!=(const rect &R) const { return _mins != R._mins || _maxs != R._maxs; }
    vec2i operator[](std::size_t idx) const { return (&_mins)[idx]; }
    vec2i& operator[](std::size_t idx) { return (&_mins)[idx]; }

    vec2i& mins() { return _mins; }
    vec2i& maxs() { return _mins; }
    vec2i mins() const { return _mins; }
    vec2i maxs() const { return _maxs; }

// algebraic vector operations

    rect operator+(vec2i const& V) const { return rect(_mins+V, _maxs+V); }
    rect operator-(vec2i const& V) const { return rect(_mins-V, _maxs-V); }
    rect operator*(int S) const { return rect(_mins*S, _maxs*S); }
    rect operator/(int S) const { return rect(_mins/S, _maxs/S); }

// boolean operations

    rect operator|(rect const& R) const {
        return rect(vec2i(std::min<int>(_mins.x, R._mins.x),
                          std::min<int>(_mins.y, R._mins.y)),
                    vec2i(std::max<int>(_maxs.x, R._maxs.x),
                          std::max<int>(_maxs.y, R._maxs.y)));
    }

    rect operator&(rect const& R) const {
        return rect(vec2i(std::max<int>(_mins.x, R._mins.x),
                          std::max<int>(_mins.y, R._mins.y)),
                    vec2i(std::min<int>(_maxs.x, R._maxs.x),
                          std::min<int>(_maxs.y, R._maxs.y)));
    }

// algebraic vector assignment operations

    rect& operator+=(vec2i const& V) { _mins += V; _maxs += V; return *this; }
    rect& operator-=(vec2i const& V) { _mins -= V; _maxs -= V; return *this; }
    rect& operator*=(int S) { _mins *= S; _maxs *= S; return *this; }
    rect& operator/=(int S) { _mins /= S; _maxs /= S; return *this; }

// boolean assignment operations

    rect& operator|=(rect const& R) { *this = *this | R; return *this; }
    rect& operator&=(rect const& R) { *this = *this & R; return *this; }

// utility functions

    vec2i center() const { return _mins + (_maxs - _mins) / 2; }
    vec2i size() const { return _maxs - _mins; }
    int area() const { return (_maxs.x - _mins.x) * (_maxs.y - _mins.y); }

    bool contains(vec2i point) const {
        return point.x >= _mins.x
            && point.y >= _mins.y
            && point.x <= _maxs.x
            && point.y <= _maxs.y;
    }

    void clear() { _mins.clear(); _maxs.clear(); }

    bool empty() const { return _maxs.x <= _mins.x || _maxs.y <= _mins.y; }
    bool inverted() const { return _maxs.x < _mins.x && _maxs.y < _mins.y; }

    static rect from_center(vec2i center, vec2i size) {
        return rect(center - size / 2, center + (size - size / 2));
    }

protected:
    vec2i _mins;
    vec2i _maxs;
};

//
//  matrices
//

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

//------------------------------------------------------------------------------
inline vec2 rotate(vec2 v, float rad)
{
    float cosa = std::cos(rad);
    float sina = std::sin(rad);

    return vec2(v.x * cosa - v.y * sina,
                v.x * sina + v.y * cosa);
}

////////////////////////////////////////////////////////////////////////////////
// matrix types

//------------------------------------------------------------------------------
class mat3
{
public:

// constructors

    mat3() = default;
    constexpr mat3(float in11, float in12, float in13,
                   float in21, float in22, float in23,
                   float in31, float in32, float in33)
        : _rows{vec3{in11, in12, in13},
                vec3{in21, in22, in23},
                vec3{in31, in32, in33}}
    {}

    mat3& operator=(mat3 const& M) {_rows[0]=M[0]; _rows[1]=M[1]; _rows[2]=M[2]; return *this;}
    bool operator==(mat3 const& M) const { return _rows[0] == M[0] && _rows[1] == M[1] && _rows[2] == M[2]; }
    bool operator!=(mat3 const& M) const { return _rows[0] != M[0] || _rows[1] != M[1] || _rows[2] != M[2]; }
    vec3 const& operator[](std::size_t idx) const { return _rows[idx]; }
    vec3& operator[](std::size_t idx) { return _rows[idx]; }

// basic functions

    void set_identity() {
        _rows[0][0] = 1.f; _rows[0][1] = 0.f; _rows[0][2] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = 1.f; _rows[1][2] = 0.f;
        _rows[2][0] = 0.f; _rows[2][1] = 0.f; _rows[2][2] = 1.f;
    }

// rotation

    template<int axis> void set_rotation(float theta) {
        float cost = std::cos(theta);
        float sint = std::sin(theta);

        constexpr int i0 = axis, i1 = (axis + 1) % 3, i2 = (axis + 2) % 3;

        _rows[i0][i0] = 1.f; _rows[i0][i1] =   0.f; _rows[i0][i2] =   0.f;
        _rows[i1][i0] = 0.f; _rows[i1][i1] = +cost; _rows[i1][i2] = -sint;
        _rows[i2][i0] = 0.f; _rows[i2][i1] = +sint; _rows[i2][i2] = +cost;
    }

// multiplication

    friend vec2 operator*(vec2 const& v, mat3 const& m) {
        return vec2(m[0][0] * v[0] + m[0][1] * v[1] + m[0][1],
                    m[1][0] * v[0] + m[1][1] * v[1] + m[1][2]);
    }

    friend vec3 operator*(vec3 const& v, mat3 const& m) {
        return vec3(m[0][0] * v[0] + m[0][1] * v[1] + m[0][1] * v[2],
                    m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2],
                    m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]);
    }

protected:
    vec3 _rows[3];
};

//------------------------------------------------------------------------------
class mat4
{
public:

// constructors

    mat4() = default;
    constexpr mat4(float in11, float in12, float in13, float in14,
                   float in21, float in22, float in23, float in24,
                   float in31, float in32, float in33, float in34,
                   float in41, float in42, float in43, float in44)
        : _rows{vec4{in11, in12, in13, in14},
                vec4{in21, in22, in23, in24},
                vec4{in31, in32, in33, in34},
                vec4{in41, in42, in43, in44}}
    {}

    mat4& operator=(mat4 const& M) {_rows[0]=M[0]; _rows[1]=M[1]; _rows[2]=M[2]; _rows[3]=M[3]; return *this;}
    bool operator==(mat4 const& M) const { return _rows[0] == M[0] && _rows[1] == M[1] && _rows[2] == M[2] && _rows[3] == M[3]; }
    bool operator!=(mat4 const& M) const { return _rows[0] != M[0] || _rows[1] != M[1] || _rows[2] != M[2] || _rows[3] != M[3]; }
    vec4 const& operator[](std::size_t idx) const { return _rows[idx]; }
    vec4& operator[](std::size_t idx) { return _rows[idx]; }

// basic functions

    void set_identity() {
        _rows[0][0] = 1.f; _rows[0][1] = 0.f; _rows[0][2] = 0.f; _rows[0][3] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = 1.f; _rows[1][2] = 0.f; _rows[1][3] = 0.f;
        _rows[2][0] = 0.f; _rows[2][1] = 0.f; _rows[2][2] = 1.f; _rows[2][3] = 0.f;
        _rows[3][0] = 0.f; _rows[3][1] = 0.f; _rows[3][2] = 0.f; _rows[3][3] = 1.f;
    }

// rotation

    template<int axis> void set_rotation(float theta) {
        float cost = std::cos(theta);
        float sint = std::sin(theta);

        constexpr int i0 = axis, i1 = (axis + 1) % 3, i2 = (axis + 2) % 3;

        _rows[i0][i0] = 1.f; _rows[i0][i1] =   0.f; _rows[i0][i2] =   0.f; _rows[ 0][ 3] = 0.f;
        _rows[i1][i0] = 0.f; _rows[i1][i1] = +cost; _rows[i1][i2] = -sint; _rows[ 1][ 3] = 0.f;
        _rows[i2][i0] = 0.f; _rows[i2][i1] = +sint; _rows[i2][i2] = +cost; _rows[ 2][ 3] = 0.f;
        _rows[ 3][ 0] = 0.f; _rows[ 3][ 1] =   0.f; _rows[ 3][ 2] =   0.f; _rows[ 3][ 3] = 1.f;
    }

// translation

    void set_translation(vec4 const& t) {
        _rows[0][0] = 1.f; _rows[0][1] = 0.f; _rows[0][2] = 0.f; _rows[0][3] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = 1.f; _rows[1][2] = 0.f; _rows[1][3] = 0.f;
        _rows[2][0] = 0.f; _rows[2][1] = 0.f; _rows[2][2] = 1.f; _rows[2][3] = 0.f;
        _rows[3][0] = t.x; _rows[3][1] = t.y; _rows[3][2] = t.z; _rows[3][3] = t.w;
    }

    void set_translation(vec3 const& t) { set_translation(vec4(t)); }

// scale

    void set_scale(vec4 const& s) {
        _rows[0][0] = s.x; _rows[0][1] = 0.f; _rows[0][2] = 0.f; _rows[0][3] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = s.y; _rows[1][2] = 0.f; _rows[1][3] = 0.f;
        _rows[2][0] = 0.f; _rows[2][1] = 0.f; _rows[2][2] = s.z; _rows[2][3] = 0.f;
        _rows[3][0] = 0.f; _rows[3][1] = 0.f; _rows[3][2] = 0.f; _rows[3][3] = s.w;
    }

    void set_scale(float s) { set_scale(vec4(s)); }

// multiplication

    friend vec3 operator*(vec3 const& v, mat4 const& m) {
        return vec3(m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3],
                    m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3],
                    m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3]);
    }

    friend vec4 operator*(vec4 const& v, mat4 const& m) {
        return vec4(m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3],
                    m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3],
                    m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3],
                    m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3]);
    }

protected:
    vec4 _rows[4];
};

/*
=============================

TYPE DEFINITIONS

=============================
*/

using byte = std::uint8_t;
using word = std::uint16_t;

//
// unit types
//

constexpr vec2 vec2_zero = vec2(0,0);
constexpr vec3 vec3_zero = vec3(0,0,0);
constexpr vec4 vec4_zero = vec4(0,0,0,0);

constexpr mat3 mat3_identity = mat3(1,0,0,0,1,0,0,0,1);
constexpr mat4 mat4_identity = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
