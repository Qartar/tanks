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

#define ROLL    0
#define PITCH   1
#define YAW     2

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
    constexpr explicit vec4(vec3 &V, float W = 1) : x(V.x), y(V.y), z(V.z), w(W) {}

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
    union
    {
        float   m[3][3];    //  [y][x]
        struct
        {           // columns then rows for OpenGL compliance
            float   _11, _21, _31;  //  0   3   6
            float   _12, _22, _32;  //  1   4   7
            float   _13, _23, _33;  //  2   5   8
        };
    };

// constructors
    inline  mat3   ()  { identity(); }
    inline  mat3   (   float in11, float in12, float in13,
                        float in21, float in22, float in23,
                        float in31, float in32, float in33  )
    {   _11 = in11 ; _12 = in12 ; _13 = in13 ;
        _21 = in21 ; _22 = in22 ; _23 = in23 ;
        _31 = in32 ; _32 = in32 ; _33 = in33 ; }

// basic functions
    inline void identity () {
        _11=1   ;   _12=0   ;   _13=0   ;
        _21=0   ;   _22=1   ;   _23=0   ;
        _31=0   ;   _32=0   ;   _33=1   ;   }

//  initializers for rotations
    inline void rotatepitch (float theta) { 
        _11=1 ; _12=0           ; _13=0             ;
        _21=0 ; _22=cosf(theta) ; _23=sinf(theta)   ;
        _31=0 ; _32=-sinf(theta); _33=cosf(theta)   ; }

    inline void rotateroll (float theta) {
        _11=cosf(theta) ;   _12=0   ;   _13=-sinf(theta);
        _21=0           ;   _22=1   ;   _23=0           ;
        _31=sinf(theta) ;   _32=0   ;   _33=cosf(theta) ; }

    inline void rotateyaw (float theta) {
        _11=cosf(theta) ;   _12=-sinf(theta);   _13=0   ;
        _21=sinf(theta) ;   _22=cosf(theta) ;   _23=0   ;
        _31=0;          ;   _32=0;          ;   _33=0   ; }

//  multiplication functions
    inline vec2 mult (vec2 in) { return ( vec2(
        _11*in[0] + _12*in[1] + _13,
        _21*in[0] + _22*in[1] + _23) ); }

    inline vec3 mult (vec3 in) { return ( vec3(
        _11*in[0] + _12*in[1] + _13*in[2],
        _21*in[0] + _22*in[1] + _23*in[2],
        _31*in[0] + _32*in[1] + _33*in[2]) ); }
};

//------------------------------------------------------------------------------
class mat4
{
public:
    union
    {
        float   m[4][4];    // [y][x]
        struct
        {           // columns then rows for OpenGL compliance
            float   _11, _21, _31, _41; //  0   4   8   12
            float   _12, _22, _32, _42; //  1   5   9   13
            float   _13, _23, _33, _43; //  2   6   10  14
            float   _14, _24, _34, _44; //  3   7   11  15
        };
    };

// constructors
    inline mat4 () { identity(); }
    inline mat4 (  float in11, float in12, float in13, float in14, 
                    float in21, float in22, float in23, float in24, 
                    float in31, float in32, float in33, float in34, 
                    float in41, float in42, float in43, float in44) 
    {   _11 = in11 ; _12 = in12 ; _13 = in13 ; _14 = in14 ;
        _21 = in21 ; _22 = in22 ; _23 = in23 ; _24 = in24 ;
        _31 = in31 ; _32 = in32 ; _33 = in33 ; _34 = in34 ;
        _41 = in41 ; _42 = in42 ; _43 = in43 ; _44 = in44 ; }

// basic functions
    inline void identity () {
        _11=1   ;   _12=0   ; _13=0 ;   _14=0   ;
        _21=0   ;   _22=1   ; _23=0 ;   _24=0   ;
        _31=0   ;   _32=0   ; _33=1 ;   _34=0   ;
        _41=0   ;   _42=0   ; _43=0 ;   _44=1   ;   }

// translation
    inline void translate (vec4 const& t) {
        _11=1   ;   _12=0   ; _13=0 ;   _14=t.x ;
        _21=0   ;   _22=1   ; _23=0 ;   _24=t.y ;
        _31=0   ;   _32=0   ; _33=1 ;   _34=t.z ;
        _41=0   ;   _42=0   ; _43=0 ;   _44=1   ;   }

// scale
    inline void scale (vec4 const& s) {
        _11=s.x ;   _12=0   ; _13=0     ;   _14=0   ;
        _21=0   ;   _22=s.y ; _23=0     ;   _24=0   ;
        _31=0   ;   _32=0   ; _33=s.z   ;   _34=0   ;
        _41=0   ;   _42=0   ; _43=0     ;   _44=1   ;   }

// rotate
    inline void rotate (int plane, float theta) {
        switch ( plane ) {
            case ROLL: {
                _11=1   ;   _12=0           ; _13=0             ;   _14=0   ;
                _21=0   ;   _22=cosf(theta) ; _23=-sinf(theta)  ;   _24=0   ;
                _31=0   ;   _32=sinf(theta) ; _33=cosf(theta)   ;   _34=0   ;
                _41=0   ;   _42=0           ; _43=0             ;   _44=1   ;   break;  }

            case PITCH: {
                _11=cosf(theta) ;   _12=0   ; _13=sinf(theta)   ;   _14=0   ;
                _21=0           ;   _22=1   ; _23=0             ;   _24=0   ;
                _31=-sinf(theta);   _32=0   ; _33=cosf(theta)   ;   _34=0   ;
                _41=0           ;   _42=0   ; _43=0             ;   _44=1   ;   break;  }

            case YAW: {
                _11=cosf(theta) ;   _12=-sinf(theta); _13=0     ;   _14=0   ;
                _21=sinf(theta) ;   _22=cosf(theta) ; _23=0     ;   _24=0   ;
                _31=0           ;   _32=0           ; _33=1     ;   _34=0   ;
                _41=0           ;   _42=0           ; _43=0     ;   _44=1   ;   break;  }

            default: {
                break;  } 
        } }

// point around arbitrary rotation
    inline vec3 rotate (vec3 &rot, vec3 &pt) {
        vec3 rad = rot*(float)(M_PI/180.0f);
        return (
            mat4(   1,           0,           0,           0,
                    0,           cosf(rad.x),-sinf(rad.x), 0,
                    0,           sinf(rad.x), cosf(rad.x), 0,
                    0,           0,           0,           1 ) *

            mat4(  cosf(rad.y), 0,           sinf(rad.y), 0,
                    0,           1,           0,           0,
                   -sinf(rad.y), 0,           cosf(rad.y), 0,
                    0,           0,           0,           1 ) *

            mat4(  cosf(rad.z),-sinf(rad.z), 0,           0,
                    sinf(rad.z), cosf(rad.z), 0,           0,
                    0,           0,           1,           0,
                    0,           0,           0,           1 ) *

            vec4(  pt.x,        pt.y,        pt.z,       1 ) ).to_vec3(); }

// set arbitrary rotation matrix
    inline void rotate (vec3 &rot) {
        vec3 rad = rot*(float)(M_PI/180.0f);
        *this = (
            mat4(  1,          0,           0,           0,
                    0,          cosf(rad.x),-sinf(rad.x), 0,
                    0,          sinf(rad.x), cosf(rad.x), 0,
                    0,          0,           0,           1 ) *

            mat4(  cosf(rad.y), 0,          sinf(rad.y), 0,
                    0,           1,          0,           0,
                   -sinf(rad.y), 0,          cosf(rad.y), 0,
                    0,           0,          0,           1 ) *

            mat4(  cosf(rad.z),-sinf(rad.z), 0,          0,
                    sinf(rad.z), cosf(rad.z), 0,          0,
                    0,           0,           1,          0,
                    0,           0,           0,          1 ) ); }



// multiply
    inline mat4 operator * (mat4 &M) {
        return mat4(   m[0][0]*M.m[0][0]+m[0][1]*M.m[1][0]+m[0][2]*M.m[2][0]+m[0][3]*M.m[3][0],    m[0][0]*M.m[0][1]+m[0][1]*M.m[1][1]+m[0][2]*M.m[2][1]+m[0][3]*M.m[3][1],    m[0][0]*M.m[0][2]+m[0][1]*M.m[1][2]+m[0][2]*M.m[2][2]+m[0][3]*M.m[3][2],    m[0][0]*M.m[0][3]+m[0][1]*M.m[1][3]+m[0][2]*M.m[2][3]+m[0][3]*M.m[3][3],
                        m[1][0]*M.m[0][0]+m[1][1]*M.m[1][0]+m[1][2]*M.m[2][0]+m[1][3]*M.m[3][0],    m[1][0]*M.m[0][1]+m[1][1]*M.m[1][1]+m[1][2]*M.m[2][1]+m[1][3]*M.m[3][1],    m[1][0]*M.m[0][2]+m[1][1]*M.m[1][2]+m[1][2]*M.m[2][2]+m[1][3]*M.m[3][2],    m[1][0]*M.m[0][3]+m[1][1]*M.m[1][3]+m[1][2]*M.m[2][3]+m[1][3]*M.m[3][3],
                        m[2][0]*M.m[0][0]+m[2][1]*M.m[1][0]+m[2][2]*M.m[2][0]+m[2][3]*M.m[3][0],    m[2][0]*M.m[0][1]+m[2][1]*M.m[1][1]+m[2][2]*M.m[2][1]+m[2][3]*M.m[3][1],    m[2][0]*M.m[0][2]+m[2][1]*M.m[1][2]+m[2][2]*M.m[2][2]+m[2][3]*M.m[3][2],    m[2][0]*M.m[0][3]+m[2][1]*M.m[1][3]+m[2][2]*M.m[2][3]+m[2][3]*M.m[3][3],
                        m[3][0]*M.m[0][0]+m[3][1]*M.m[1][0]+m[3][2]*M.m[2][0]+m[3][3]*M.m[3][0],    m[3][0]*M.m[0][1]+m[3][1]*M.m[1][1]+m[3][2]*M.m[2][1]+m[3][3]*M.m[3][1],    m[3][0]*M.m[0][2]+m[3][1]*M.m[1][2]+m[3][2]*M.m[2][2]+m[3][3]*M.m[3][2],    m[3][0]*M.m[0][3]+m[3][1]*M.m[1][3]+m[3][2]*M.m[2][3]+m[3][3]*M.m[3][3] ); }

    inline mat4 &operator *= (mat4 &M) {
        _11 = m[0][0]*M.m[0][0]+m[0][1]*M.m[1][0]+m[0][2]*M.m[2][0]+m[0][3]*M.m[3][0]   ;   _12 = m[0][0]*M.m[0][1]+m[0][1]*M.m[1][1]+m[0][2]*M.m[2][1]+m[0][3]*M.m[3][1]   ;   _13 = m[0][0]*M.m[0][2]+m[0][1]*M.m[1][2]+m[0][2]*M.m[2][2]+m[0][3]*M.m[3][2]   ;   _14 = m[0][0]*M.m[0][3]+m[0][1]*M.m[1][3]+m[0][2]*M.m[2][3]+m[0][3]*M.m[3][3]   ;
        _21 = m[1][0]*M.m[0][0]+m[1][1]*M.m[1][0]+m[1][2]*M.m[2][0]+m[1][3]*M.m[3][0]   ;   _22 = m[1][0]*M.m[0][1]+m[1][1]*M.m[1][1]+m[1][2]*M.m[2][1]+m[1][3]*M.m[3][1]   ;   _23 = m[1][0]*M.m[0][2]+m[1][1]*M.m[1][2]+m[1][2]*M.m[2][2]+m[1][3]*M.m[3][2]   ;   _24 = m[1][0]*M.m[0][3]+m[1][1]*M.m[1][3]+m[1][2]*M.m[2][3]+m[1][3]*M.m[3][3]   ;
        _31 = m[2][0]*M.m[0][0]+m[2][1]*M.m[1][0]+m[2][2]*M.m[2][0]+m[2][3]*M.m[3][0]   ;   _32 = m[2][0]*M.m[0][1]+m[2][1]*M.m[1][1]+m[2][2]*M.m[2][1]+m[2][3]*M.m[3][1]   ;   _33 = m[2][0]*M.m[0][2]+m[2][1]*M.m[1][2]+m[2][2]*M.m[2][2]+m[2][3]*M.m[3][2]   ;   _34 = m[2][0]*M.m[0][3]+m[2][1]*M.m[1][3]+m[2][2]*M.m[2][3]+m[2][3]*M.m[3][3]   ;
        _41 = m[3][0]*M.m[0][0]+m[3][1]*M.m[1][0]+m[3][2]*M.m[2][0]+m[3][3]*M.m[3][0]   ;   _42 = m[3][0]*M.m[0][1]+m[3][1]*M.m[1][1]+m[3][2]*M.m[2][1]+m[3][3]*M.m[3][1]   ;   _43 = m[3][0]*M.m[0][2]+m[3][1]*M.m[1][2]+m[3][2]*M.m[2][2]+m[3][3]*M.m[3][2]   ;   _44 = m[3][0]*M.m[0][3]+m[3][1]*M.m[1][3]+m[3][2]*M.m[2][3]+m[3][3]*M.m[3][3]   ;   }

// multiply vector
    inline vec4 operator*(vec4 const& V) {
        return vec4(m[0][0]*V[0] + m[0][1]*V[1] + m[0][2]*V[2] + m[0][3]*V[3],
                    m[1][0]*V[0] + m[1][1]*V[1] + m[1][2]*V[2] + m[1][3]*V[3],
                    m[2][0]*V[0] + m[2][1]*V[1] + m[2][2]*V[2] + m[2][3]*V[3],
                    m[3][0]*V[0] + m[3][1]*V[1] + m[3][2]*V[2] + m[3][3]*V[3]);
    }
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

const mat3 mat3_identity = mat3(1,0,0,0,1,0,0,0,1);
const mat4 mat4_identity = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
