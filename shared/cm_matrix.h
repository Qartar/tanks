// cm_matrix.h
//

#pragma once

#include "cm_vector.h"

////////////////////////////////////////////////////////////////////////////////
// matrix types

//------------------------------------------------------------------------------
class mat2
{
public:

// constructors

    mat2() = default;
    constexpr mat2(float in11, float in12,
                   float in21, float in22)
        : _rows{vec2{in11, in12},
                vec2{in21, in22}}
    {}

    mat2& operator=(mat2 const& M) {_rows[0]=M[0]; _rows[1]=M[1]; return *this;}
    bool operator==(mat2 const& M) const { return _rows[0] == M[0] && _rows[1] == M[1]; }
    bool operator!=(mat2 const& M) const { return _rows[0] != M[0] || _rows[1] != M[1]; }
    vec2 const& operator[](std::size_t idx) const { return _rows[idx]; }
    vec2& operator[](std::size_t idx) { return _rows[idx]; }

// basic functions

    void set_identity() {
        _rows[0][0] = 1.f; _rows[0][1] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = 1.f;
    }

// rotation

    void set_rotation(float theta) {
        float cost = std::cos(theta);
        float sint = std::sin(theta);

        _rows[0][0] = +cost; _rows[0][1] = -sint;
        _rows[1][0] = +sint; _rows[1][1] = +cost;
    }

// scale

    void set_scale(vec2 const& s) {
        _rows[0][0] = s.x; _rows[0][1] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = s.y;
    }

    void set_scale(float s) { set_scale(vec2(s)); }

// multiplication

    friend vec2 operator*(vec2 const& v, mat2 const& m) {
        return vec2(m[0][0] * v[0] + m[0][1] * v[1],
                    m[1][0] * v[0] + m[1][1] * v[1]);
    }

protected:
    vec2 _rows[2];
};

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

// scale

    void set_scale(vec3 const& s) {
        _rows[0][0] = s.x; _rows[0][1] = 0.f; _rows[0][2] = 0.f;
        _rows[1][0] = 0.f; _rows[1][1] = s.y; _rows[1][2] = 0.f;
        _rows[2][0] = 0.f; _rows[2][1] = 0.f; _rows[2][2] = s.z;
    }

    void set_scale(float s) { set_scale(vec3(s)); }

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

//------------------------------------------------------------------------------
constexpr mat2 mat2_identity = mat2(1,0,0,1);
constexpr mat3 mat3_identity = mat3(1,0,0,0,1,0,0,0,1);
constexpr mat4 mat4_identity = mat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
