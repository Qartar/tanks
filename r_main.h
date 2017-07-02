/*
===============================================================================

Name    :   r_main.h

Purpose :   Rendering controller

Date    :   10/19/2004

===============================================================================
*/

#pragma once

#define MAX_FONTS   16
#define NUM_CHARS   256

/*
===========================================================

Name    :   cFont (r_font.cpp)

Purpose :   OpenGL Font Encapsulation

===========================================================
*/

typedef int rfont_t;
class cFont
{
public:
    cFont () {}
    ~cFont () {}

    int     Init (char *szName, int nSize, unsigned int bitFlags);
    int     Shutdown ();

    bool    Compare (char *szName, int nSize, unsigned int bitFlags);
    HFONT   Activate ();

    void    Draw (char *szString, vec2 vPos, vec4 vColor);

    bool    is_inuse () { return (m_hFont != NULL); }

private:
    HFONT       m_hFont;
    unsigned    m_listBase;

    byte        m_width[NUM_CHARS];

    char        m_szName[64];
    int         m_nSize;
    unsigned int    m_bitFlags;
};

/*
===========================================================

Name    :   cRender (r_main.cpp)

Purpose :   Rendering controller object

===========================================================
*/

class cModel;

typedef int rimage_t;

class cRender
{
public:
    cRender () {}
    ~cRender () {}

    int     Init ();
    int     Shutdown ();

    void    BeginFrame ();
    void    EndFrame ();

    void    Resize () { m_setDefaultState( ) ; }

    // Font Interface (r_font.cpp)

    rfont_t AddFont (char *szName, int nSize, unsigned int bitFlags);
    int     RemoveFont (rfont_t hFont);
    rfont_t UseFont (rfont_t hFont);

    //  Image Interface (r_image.cpp)
    rimage_t    LoadImage( const char *szFilename );
    void        DrawImage( rimage_t img, vec2 org, vec2 sz, vec4 color );

    // Drawing Functions (r_draw.cpp)

    void    DrawString (char *szString, vec2 vPos, vec4 vColor);
    void    DrawLine (vec2 vOrg, vec2 vEnd, vec4 vColorO, vec4 vColorE);
    void    DrawBox (vec2 vSize, vec2 vPos, float flAngle, vec4 vColor);
    void    DrawModel (cModel *pModel, vec2 vPos, float flAngle, vec4 vColor);
    void    DrawParticles (float time, render::particle const* particles, std::size_t num_particles);

    void    SetViewOrigin (vec2 vPos) { m_viewOrigin = vPos ; m_setDefaultState( ) ; }

private:

    // More font stuff (r_font.cpp)

    void    m_InitFonts ();
    void    m_ClearFonts ();
    HFONT   m_sysFont;
    cFont   m_Fonts[MAX_FONTS];
    rfont_t m_activeFont;

    // Internal stuff

    void    m_setDefaultState ();

    vec2    m_viewOrigin;

    bool    m_bATI;

    float   costbl[360];
    float   sintbl[360];
};
