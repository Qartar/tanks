/*
===========================================================

Name    :   oed_types.h

Purpose :   data type definitions

Modified:   11/03/2006

===========================================================
*/

#ifndef __OED_TYPES__
#define __OED_TYPES__

#include <math.h>   // sqrt

#define ROLL    0
#define PITCH   1
#define YAW     2

/*
===========================================================

CLASS DEFINITIONS

===========================================================
*/

class cVec2;
class cVec3;
class cVec4;
class cRay2;
class cLine2;
class cMat3;
class cMat4;

//
// class definitions for vectors
//

class cVec2
{
public:
    union
    {
        float   v[2];
        struct
        {
            float   x;
            float   y;
        };
    };

// constructors
    inline          cVec2   ()                  {}
    __forceinline   cVec2   (float X, float Y)  {   x = X   ;   y = Y   ;   }

    __forceinline   cVec2   &operator=  (const cVec2 &V) {x=V.x;y=V.y;return *this;}
    __forceinline   cVec2   operator-   (void)           const {return cVec2(-x,-y);}
    __forceinline   bool    operator==  (const cVec2 &V) const {return(x==V.x && y==V.y);}
    __forceinline   bool    operator!=  (const cVec2 &V) const {return(!(*this==V));}
    __forceinline   float   operator[]  (unsigned int n) const {return v[n];}
    __forceinline   operator float*     ()                     {return v;}
    __forceinline   operator const float*   ()           const {return v;}

// vector operators
    __forceinline   cVec2   operator+ (const cVec2 &V) const { return cVec2(   x+V.x   ,   y+V.y   );  }
    __forceinline   cVec2   operator- (const cVec2 &V) const { return cVec2(   x-V.x   ,   y-V.y   );  }
    __forceinline   cVec2   operator* (const cVec2 &V) const { return cVec2(   x*V.x   ,   y*V.y   );  }
    __forceinline   cVec2   operator/ (const cVec2 &V) const { return cVec2(   x/V.x   ,   y/V.y   );  }

// implicit operators
    __forceinline   cVec2   &operator+=(const cVec2 &V) {   x+=V.x  ;   y+=V.y  ;   return*this;    }
    __forceinline   cVec2   &operator-=(const cVec2 &V) {   x-=V.x  ;   y-=V.y  ;   return*this;    }
    __forceinline   cVec2   &operator*=(const cVec2 &V) {   x*=V.x  ;   y*=V.y  ;   return*this;    }
    __forceinline   cVec2   &operator/=(const cVec2 &V) {   x/=V.x  ;   y/=V.y  ;   return*this;    }

// linear operators
    __forceinline   cVec2   operator* (float L) const { return cVec2(   x*L ,   y*L );  }
    __forceinline   cVec2   operator/ (float L) const { return cVec2(   x/L ,   y/L );  }

//  linear implicit
    __forceinline   cVec2   &operator*=(float L)    {   x*=L    ;   y*=L    ;   return*this;    }
    __forceinline   cVec2   &operator/=(float L)    {   x/=L    ;   y/=L    ;   return*this;    }

// utility functions
    __forceinline   float   length()    const {   return(sqrtf(x*x+y*y));  }
    __forceinline   float   lengthsq()  const {   return(x*x+y*y);        }
    __forceinline   cVec2   &normalize()      {   (*this/=length());return*this;}
    __forceinline   void    clear()           {   x=0.0f  ;   y=0.0f  ;   }

    __forceinline   float   dot     (const cVec2 &V) const {   return(x*V.x+y*V.y);    }
    __forceinline   cVec2   cross   (float V)        const {   return cVec2(y*V,-x*V); }
};

class cVec3
{
public:
    union
    {
        float   v[3];
        struct
        {
            float   x;
            float   y;
            float   z;
        };
    };

// constructors
    inline          cVec3   ()                          {}
    __forceinline   cVec3   (float X, float Y, float Z) {x=X;y=Y;z=Z;}
    __forceinline   cVec3   (cVec2& V, float Z = 0) {x=V.x;y=V.y;z=Z;}
    __forceinline   cVec3   (cVec4 &V) { *this = *(cVec3 *)(&V); }

    __forceinline   cVec3   &operator=  (const cVec3 &V) {x=V.x;y=V.y;z=V.z;return *this;}
    __forceinline   cVec3   operator-   (void)           const {return cVec3(-x,-y,-z);}
    __forceinline   bool    operator==  (const cVec3 &V) const {return(x==V.x && y==V.y && z==V.z);}
    __forceinline   bool    operator!=  (const cVec3 &V) const {return(!(*this==V));}
    __forceinline   float   operator[]  (unsigned int n) const {return v[n];}
    __forceinline   operator float*     ()                     {return v;}
    __forceinline   operator const float*   ()           const {return v;}

// vector operations
    __forceinline   cVec3   operator+   (const cVec3 &V) const {return cVec3(x+V.x,y+V.y,z+V.z);}
    __forceinline   cVec3   operator-   (const cVec3 &V) const {return cVec3(x-V.x,y-V.y,z-V.z);}
    __forceinline   cVec3   operator*   (const cVec3 &V) const {return cVec3(x*V.x,y*V.y,z*V.z);}
    __forceinline   cVec3   operator/   (const cVec3 &V) const {return cVec3(x/V.x,y/V.y,z/V.z);}

// implicit operations
    __forceinline   cVec3   &operator+= (const cVec3 &V)    {x+=V.x;y+=V.y;z+=V.z;return*this;}
    __forceinline   cVec3   &operator-= (const cVec3 &V)    {x-=V.x;y-=V.y;z-=V.z;return*this;}
    __forceinline   cVec3   &operator*= (const cVec3 &V)    {x*=V.x;y*=V.y;z*=V.z;return*this;}
    __forceinline   cVec3   &operator/= (const cVec3 &V)    {x/=V.x;y/=V.y;z/=V.z;return*this;}

// linear operations
    __forceinline   cVec3   operator*   (float L) const {return cVec3(x*L,y*L,z*L);}
    __forceinline   cVec3   operator/   (float L) const {return cVec3(x/L,y/L,z/L);}

// linear implicit
    __forceinline   cVec3   &operator*= (float L)   {x*=L;y*=L;z*=L;return*this;}
    __forceinline   cVec3   &operator/= (float L)   {x/=L;y/=L;z/=L;return*this;}

// utility functions
    __forceinline   float   length      () const {return sqrtf(x*x+y*y+z*z);}
    __forceinline   float   lengthsq    () const {return (x*x+y*y+z*z);}
    __forceinline   cVec3   &normalize  ()       {(*this/=length());return*this;}
    __forceinline   void    clear       ()       {x=0.0f;y=0.0f;z=0.0f;}

    __forceinline   float   dot         (const cVec3 &V) const {return (x*V.x+y*V.y+z*V.z);}
    __forceinline   cVec3   cross       (const cVec3 &V) const {return cVec3(  y*V.z - z*V.y,  z*V.x - x*V.z,  x*V.y - y*V.x );    }

    __forceinline   cVec2   to_vec2     () const {return cVec2(x,y);}
};

class cVec4
{
public:
    union
    {
        float   v[4];
        struct
        {   // 3d homogenous coordinates
            float   x;
            float   y;
            float   z;
            float   w;
        };
        struct
        {
            float   r;
            float   g;
            float   b;
            float   a;
        };
    };

// constructors
    inline          cVec4   ()  {}
    __forceinline   cVec4   (float X, float Y, float Z) : x(X), y(Y), z(Z), w(1) {}
    __forceinline   cVec4   (float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
    __forceinline   cVec4   (cVec3 &V) { v[0] = V.v[0] ; v[1] = V.v[1] ; v[2] = V.v[2] ; v[3] = 1.0f; }

    __forceinline   cVec4   &operator=  (const cVec4 &V) {x=V.x;y=V.y;z=V.z;w=V.w;return *this;}
    __forceinline   cVec4   operator-   (void)           const {return cVec4(-x,-y,-z);}
    __forceinline   bool    operator==  (const cVec4 &V) const {return(x==V.x && y==V.y && z==V.z && w==V.w);}
    __forceinline   bool    operator!=  (const cVec4 &V) const {return(!(*this==V));}
    __forceinline   float   operator[]  (unsigned int n) const {return v[n];}
    __forceinline   operator float*     ()                     {return v;}
    __forceinline   operator const float*   ()           const {return v;}

// vector operations
    __forceinline   cVec4   operator+   (const cVec4 &V) const {return cVec4(x+V.x,y+V.y,z+V.z,1);}
    __forceinline   cVec4   operator-   (const cVec4 &V) const {return cVec4(x-V.x,y-V.y,z-V.z,1);}
    __forceinline   cVec4   operator*   (const cVec4 &V) const {return cVec4(x*V.x,y*V.y,z*V.z,1);}
    __forceinline   cVec4   operator/   (const cVec4 &V) const {return cVec4(x/V.x,y/V.y,z/V.z,1);}

// implicit operations
    __forceinline   cVec4   &operator+= (const cVec4 &V)       {x+=V.x;y+=V.y;z+=V.z;return*this;}
    __forceinline   cVec4   &operator-= (const cVec4 &V)       {x-=V.x;y-=V.y;z-=V.z;return*this;}
    __forceinline   cVec4   &operator*= (const cVec4 &V)       {x*=V.x;y*=V.y;z*=V.z;return*this;}
    __forceinline   cVec4   &operator/= (const cVec4 &V)       {x/=V.x;y/=V.y;z/=V.z;return*this;}

// linear operations
    __forceinline   cVec4   operator*   (float L) const {return cVec4(x*L,y*L,z*L,w);}
    __forceinline   cVec4   operator/   (float L) const {return cVec4(x/L,y/L,z/L,w);}

// linear implicit
    __forceinline   cVec4   &operator*= (float L)   {x*=L;y*=L;z*=L;return*this;}
    __forceinline   cVec4   &operator/= (float L)   {x/=L;y/=L;z/=L;return*this;}

// utility functions
    __forceinline   float   length      () const {return sqrtf(x*x+y*y+z*z);}
    __forceinline   float   lengthsq    () const {return (x*x+y*y+z*z);}
    __forceinline   cVec4   &normalize  ()       {(*this/=length());return*this;}
    __forceinline   void    clear       ()       {x=0.0f;y=0.0f;z=0.0f;w=1.0f;}

    __forceinline   float   dot         (const cVec4 &V) const {return (x*V.x+y*V.y+z*V.z);}
    __forceinline   cVec4   cross       (const cVec4 &V) const {return cVec4(  y*V.z - z*V.y,  z*V.x - x*V.z,  x*V.y - y*V.x, 0.0f );  }
};

//
//  lines, rays and segments
//

class cRay2
{
public:
    cVec2   a;
    cVec2   b;

    inline  cRay2   () { a=cVec2(0,0);b=cVec2(0,0); }
    inline  cRay2   (cVec2 A, cVec2 B) : a(A), b(B) {}

    __forceinline   void    set (cVec2 A, cVec2 B) { a = A; b = B; }
    bool    intersect (const cRay2 &R, cVec2 *P = 0)    {
        float   ua, ub, d;

        d = (R.b.y - R.a.y)*(b.x - a.x) - (R.b.x - R.a.x)*(b.y - a.y);

        if ( d == 0 ) { return false; } // parallel

        ua = ((R.b.x - R.a.x)*(a.y - R.a.y) - (R.b.y - R.a.y)*(a.x - R.a.x)) / d;
        ub = ((b.x - a.x)*(a.y - R.a.y) - (b.y - a.y)*(a.x - R.a.x)) / d;

        if ( ua < 0 || ub < 0 )
            return false;

        if ( P )
        {
            P->x = a.x + ua*(b.x - a.x);
            P->y = a.y + ua*(b.y - a.y);
        }

        return true;
    }
};

class cLine2
{
public:
    cVec2   a;
    cVec2   b;

    bool    is_segment;

    inline  cLine2  ()  { a=cVec2(0,0);b=cVec2(0,0); is_segment = false; }
    inline  cLine2  (cVec2 A, cVec2 B, bool S = false) : a(A), b(B), is_segment(S) {}

    __forceinline   void    set (cVec2 A, cVec2 B, bool S = false) { a = A; b = B; is_segment = S; }

    bool    intersect   (const cLine2 &L, cVec2 *P = 0) {
        float   ua, ub, d;

        d = (L.b.y - L.a.y)*(b.x - a.x) - (L.b.x - L.a.x)*(b.y - a.y);

        if ( d == 0 ) { return false; } // parallel

        ua = ((L.b.x - L.a.x)*(a.y - L.a.y) - (L.b.y - L.a.y)*(a.x - L.a.x)) / d;

        if ( is_segment && ( ua < 0 || ua > 1 ) )
            return false;

        if ( L.is_segment )
        {
            ub = ((b.x - a.x)*(a.y - L.a.y) - (b.y - a.y)*(a.x - L.a.x)) / d;

            if ( ub < 0 || ub > 1 )
                return false;
        }

        if ( P )
        {
            P->x = a.x + ua*(b.x - a.x);
            P->y = a.y + ua*(b.y - a.y);
        }

        return true;
    }

    bool    intersect   (const cRay2 &R, cVec2 *P = 0)  {
        float   ua, ub, d;

        d = (R.b.y - R.a.y)*(b.x - a.x) - (R.b.x - R.a.x)*(b.y - a.y);

        if ( d == 0 ) { return false; } // parallel

        ua = ((R.b.x - R.a.x)*(a.y - R.a.y) - (R.b.y - R.a.y)*(a.x - R.a.x)) / d;
        ub = ((b.x - a.x)*(a.y - R.a.y) - (b.y - a.y)*(a.x - R.a.x)) / d;

        if ( is_segment && ( ua < 0 || ua > 1 ) )
            return false;

        if ( ub < 0 )
            return false;

        if ( P )
        {
            P->x = a.x + ua*(b.x - a.x);
            P->y = a.y + ua*(b.y - a.y);
        }

        return true;
    }
};

class cLine3
{
public:
    cVec3   a;
    cVec3   b;

    bool    is_segment;

    inline  cLine3 () { a=cVec3(0,0,0);b=cVec3(0,0,0); is_segment = false; }
    inline  cLine3 (cVec3 A, cVec3 B, bool S = false) : a(A), b(B), is_segment(S) {}
};

class cLine4
{
public:
    cVec4   a;
    cVec4   b;

    bool    is_segment;

    inline  cLine4 () { a=cVec4(0,0,0,1);b=cVec4(0,0,0,1); is_segment = false; }
    inline  cLine4 (cVec4 A, cVec4 B, bool S = false) : a(A), b(B), is_segment(S) {}
};

//
//  matrices
//

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

class cMat3
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
    inline  cMat3   ()  { identity(); }
    inline  cMat3   (   float in11, float in12, float in13,
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
    inline cVec2 mult (cVec2 in) { return ( cVec2(
        _11*in.v[0] + _12*in.v[1] + _13,
        _21*in.v[0] + _22*in.v[1] + _23) ); }

    inline cVec3 mult (cVec3 in) { return ( cVec3(
        _11*in.v[0] + _12*in.v[1] + _13*in.v[2],
        _21*in.v[0] + _22*in.v[1] + _23*in.v[2],
        _31*in.v[0] + _32*in.v[1] + _33*in.v[2]) ); }
};

class cMat4
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
    inline cMat4 () { identity(); }
    inline cMat4 (  float in11, float in12, float in13, float in14, 
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
    inline void translate (cVec4 t) {
        _11=1   ;   _12=0   ; _13=0 ;   _14=t.x ;
        _21=0   ;   _22=1   ; _23=0 ;   _24=t.y ;
        _31=0   ;   _32=0   ; _33=1 ;   _34=t.z ;
        _41=0   ;   _42=0   ; _43=0 ;   _44=1   ;   }

// scale
    inline void scale (cVec4 s) {
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
    inline cVec3 rotate (cVec3 &rot, cVec3 &pt) {
        cVec3 rad = rot*(float)(M_PI/180.0f);
        return cVec3(
            cMat4(  1,           0,           0,           0,
                    0,           cosf(rad.x),-sinf(rad.x), 0,
                    0,           sinf(rad.x), cosf(rad.x), 0,
                    0,           0,           0,           1 ) *

            cMat4(  cosf(rad.y), 0,           sinf(rad.y), 0,
                    0,           1,           0,           0,
                   -sinf(rad.y), 0,           cosf(rad.y), 0,
                    0,           0,           0,           1 ) *

            cMat4(  cosf(rad.z),-sinf(rad.z), 0,           0,
                    sinf(rad.z), cosf(rad.z), 0,           0,
                    0,           0,           1,           0,
                    0,           0,           0,           1 ) *

            cVec4(  pt.x,        pt.y,        pt.z,       1 ) ); }

// set arbitrary rotation matrix
    inline void rotate (cVec3 &rot) {
        cVec3 rad = rot*(float)(M_PI/180.0f);
        *this = (
            cMat4(  1,          0,           0,           0,
                    0,          cosf(rad.x),-sinf(rad.x), 0,
                    0,          sinf(rad.x), cosf(rad.x), 0,
                    0,          0,           0,           1 ) *

            cMat4(  cosf(rad.y), 0,          sinf(rad.y), 0,
                    0,           1,          0,           0,
                   -sinf(rad.y), 0,          cosf(rad.y), 0,
                    0,           0,          0,           1 ) *

            cMat4(  cosf(rad.z),-sinf(rad.z), 0,          0,
                    sinf(rad.z), cosf(rad.z), 0,          0,
                    0,           0,           1,          0,
                    0,           0,           0,          1 ) ); }



// multiply
    inline cMat4 operator * (cMat4 &M) {
        return cMat4(   m[0][0]*M.m[0][0]+m[0][1]*M.m[1][0]+m[0][2]*M.m[2][0]+m[0][3]*M.m[3][0],    m[0][0]*M.m[0][1]+m[0][1]*M.m[1][1]+m[0][2]*M.m[2][1]+m[0][3]*M.m[3][1],    m[0][0]*M.m[0][2]+m[0][1]*M.m[1][2]+m[0][2]*M.m[2][2]+m[0][3]*M.m[3][2],    m[0][0]*M.m[0][3]+m[0][1]*M.m[1][3]+m[0][2]*M.m[2][3]+m[0][3]*M.m[3][3],
                        m[1][0]*M.m[0][0]+m[1][1]*M.m[1][0]+m[1][2]*M.m[2][0]+m[1][3]*M.m[3][0],    m[1][0]*M.m[0][1]+m[1][1]*M.m[1][1]+m[1][2]*M.m[2][1]+m[1][3]*M.m[3][1],    m[1][0]*M.m[0][2]+m[1][1]*M.m[1][2]+m[1][2]*M.m[2][2]+m[1][3]*M.m[3][2],    m[1][0]*M.m[0][3]+m[1][1]*M.m[1][3]+m[1][2]*M.m[2][3]+m[1][3]*M.m[3][3],
                        m[2][0]*M.m[0][0]+m[2][1]*M.m[1][0]+m[2][2]*M.m[2][0]+m[2][3]*M.m[3][0],    m[2][0]*M.m[0][1]+m[2][1]*M.m[1][1]+m[2][2]*M.m[2][1]+m[2][3]*M.m[3][1],    m[2][0]*M.m[0][2]+m[2][1]*M.m[1][2]+m[2][2]*M.m[2][2]+m[2][3]*M.m[3][2],    m[2][0]*M.m[0][3]+m[2][1]*M.m[1][3]+m[2][2]*M.m[2][3]+m[2][3]*M.m[3][3],
                        m[3][0]*M.m[0][0]+m[3][1]*M.m[1][0]+m[3][2]*M.m[2][0]+m[3][3]*M.m[3][0],    m[3][0]*M.m[0][1]+m[3][1]*M.m[1][1]+m[3][2]*M.m[2][1]+m[3][3]*M.m[3][1],    m[3][0]*M.m[0][2]+m[3][1]*M.m[1][2]+m[3][2]*M.m[2][2]+m[3][3]*M.m[3][2],    m[3][0]*M.m[0][3]+m[3][1]*M.m[1][3]+m[3][2]*M.m[2][3]+m[3][3]*M.m[3][3] ); }

    inline cMat4 &operator *= (cMat4 &M) {
        _11 = m[0][0]*M.m[0][0]+m[0][1]*M.m[1][0]+m[0][2]*M.m[2][0]+m[0][3]*M.m[3][0]   ;   _12 = m[0][0]*M.m[0][1]+m[0][1]*M.m[1][1]+m[0][2]*M.m[2][1]+m[0][3]*M.m[3][1]   ;   _13 = m[0][0]*M.m[0][2]+m[0][1]*M.m[1][2]+m[0][2]*M.m[2][2]+m[0][3]*M.m[3][2]   ;   _14 = m[0][0]*M.m[0][3]+m[0][1]*M.m[1][3]+m[0][2]*M.m[2][3]+m[0][3]*M.m[3][3]   ;
        _21 = m[1][0]*M.m[0][0]+m[1][1]*M.m[1][0]+m[1][2]*M.m[2][0]+m[1][3]*M.m[3][0]   ;   _22 = m[1][0]*M.m[0][1]+m[1][1]*M.m[1][1]+m[1][2]*M.m[2][1]+m[1][3]*M.m[3][1]   ;   _23 = m[1][0]*M.m[0][2]+m[1][1]*M.m[1][2]+m[1][2]*M.m[2][2]+m[1][3]*M.m[3][2]   ;   _24 = m[1][0]*M.m[0][3]+m[1][1]*M.m[1][3]+m[1][2]*M.m[2][3]+m[1][3]*M.m[3][3]   ;
        _31 = m[2][0]*M.m[0][0]+m[2][1]*M.m[1][0]+m[2][2]*M.m[2][0]+m[2][3]*M.m[3][0]   ;   _32 = m[2][0]*M.m[0][1]+m[2][1]*M.m[1][1]+m[2][2]*M.m[2][1]+m[2][3]*M.m[3][1]   ;   _33 = m[2][0]*M.m[0][2]+m[2][1]*M.m[1][2]+m[2][2]*M.m[2][2]+m[2][3]*M.m[3][2]   ;   _34 = m[2][0]*M.m[0][3]+m[2][1]*M.m[1][3]+m[2][2]*M.m[2][3]+m[2][3]*M.m[3][3]   ;
        _41 = m[3][0]*M.m[0][0]+m[3][1]*M.m[1][0]+m[3][2]*M.m[2][0]+m[3][3]*M.m[3][0]   ;   _42 = m[3][0]*M.m[0][1]+m[3][1]*M.m[1][1]+m[3][2]*M.m[2][1]+m[3][3]*M.m[3][1]   ;   _43 = m[3][0]*M.m[0][2]+m[3][1]*M.m[1][2]+m[3][2]*M.m[2][2]+m[3][3]*M.m[3][2]   ;   _44 = m[3][0]*M.m[0][3]+m[3][1]*M.m[1][3]+m[3][2]*M.m[2][3]+m[3][3]*M.m[3][3]   ;   }

// multiply vector
    inline cVec4 operator * (cVec4 &V) {
        return cVec4(   V.v[0]*m[0][0]+V.v[1]*m[0][1]+V.v[2]*m[0][2]+V.v[3]*m[0][3],
                        V.v[0]*m[1][0]+V.v[1]*m[1][1]+V.v[2]*m[1][2]+V.v[3]*m[1][3],
                        V.v[0]*m[2][0]+V.v[1]*m[2][1]+V.v[2]*m[2][2]+V.v[3]*m[2][3],
                        V.v[0]*m[3][0]+V.v[1]*m[3][1]+V.v[2]*m[3][2]+V.v[3]*m[3][3] );  }
};

/*
=============================

TYPE DEFINITIONS

=============================
*/

// floating point vector types
using vec4 = cVec4;                 //  v[4] / x y z w / r g b a
using vec3 = cVec3;                 //  v[3] / x y z
using vec2 = cVec2;                 //  v[2] / x y

// floating point matrix types
using mat4 = cMat4;                 //  m[4][4]
using mat3 = cMat3;                 //  m[3][3]

typedef unsigned char byte;

//
// unit types
//

const vec4 vec4_unit = cVec4(1,1,1,1);
const vec3 vec3_unit = cVec3(1,1,1);
const vec2 vec2_unit = cVec2(1,1);

const vec4 vec4_null = cVec4(0,0,0,0);
const vec3 vec3_null = cVec3(0,0,0);
const vec2 vec2_null = cVec2(0,0);

const mat4 mat4_unit = cMat4(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1);
const mat3 mat3_unit = cMat3(1,0,0,0,1,0,0,0,1);

#endif // __OED_TYPES__
