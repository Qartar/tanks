// r_font.cpp
//

#include "local.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace render {

HFONT font::_system_font = NULL;
HFONT font::_active_font = NULL;

//------------------------------------------------------------------------------
render::font const* system::load_font(char const* name, int size)
{
    for (auto const& f : _fonts) {
        if (f->compare(name, size)) {
            return f.get();
        }
    }
    _fonts.push_back(std::make_unique<render::font>(name, size));
    return _fonts.back().get();
}

//------------------------------------------------------------------------------
font::font(char const* name, int size)
    : _name(name)
    , _size(size)
    , _handle(NULL)
    , _list_base(0)
    , _char_width{0}
{
    GLYPHMETRICS gm;
    MAT2 m;

    // allocate lists
    _list_base = glGenLists(kNumChars);

    // create font
    _handle = CreateFontA(
        _size,                      // cHeight
        0,                          // cWidth
        0,                          // cEscapement
        0,                          // cOrientation
        FW_NORMAL,                  // cWeight
        FALSE,                      // bItalic
        FALSE,                      // bUnderline
        FALSE,                      // bStrikeOut
        ANSI_CHARSET,               // iCharSet
        OUT_TT_PRECIS,              // iOutPrecision
        CLIP_DEFAULT_PRECIS,        // iClipPrecision
        ANTIALIASED_QUALITY,        // iQuality
        FF_DONTCARE|DEFAULT_PITCH,  // iPitchAndFamily
        _name.c_str()               // pszFaceName
    );

    // set our new font to the system
    HFONT prev_font = (HFONT )SelectObject(g_Application->window()->hdc(), _handle);

    // generate font bitmaps with selected HFONT
    memset( &m, 0, sizeof(m) );
    m.eM11.value = 1;
    m.eM12.value = 0;
    m.eM21.value = 0;
    m.eM22.value = 1;

    wglUseFontBitmapsA(g_Application->window()->hdc(), 0, kNumChars-1, _list_base);
    for (int ii = 0; ii < kNumChars; ++ii) {
        GetGlyphOutlineA(g_Application->window()->hdc(), ii, GGO_METRICS, &gm, 0, NULL, &m);
        _char_width[ii] = gm.gmCellIncX;
    }

    // restore previous font
    SelectObject(g_Application->window()->hdc(), prev_font);
}

//------------------------------------------------------------------------------
font::~font()
{
    // restore system font if this is the active font
    if (_active_font == _handle) {
        glListBase(0);
        SelectObject(g_Application->window()->hdc(), _system_font);

        _active_font = _system_font;
        _system_font = NULL;
    }

    // delete from opengl
    if (_list_base) {
        glDeleteLists(_list_base, kNumChars);
    }

    // delete font from gdi
    if (_handle) {
        DeleteObject(_handle);
    }
}

//------------------------------------------------------------------------------
bool font::compare(char const* name, int size) const
{
    return _name == name
        && _size == size;
}

//------------------------------------------------------------------------------
void font::draw(char const* string, vec2 position, color4 color) const
{
    // activate font if it isn't already
    if (_active_font != _handle) {
        HFONT prev_font = (HFONT )SelectObject(g_Application->window()->hdc(), _handle);

        // keep track of the system font so it can be restored later
        if (_system_font == NULL) {
            _system_font = prev_font;
        }

        glListBase(_list_base);
        _active_font = _handle;
    }

    int xoffs = 0;

    int r = color.r * 255;
    int g = color.g * 255;
    int b = color.b * 255;
    int a = color.a * 255;

    char const* cursor = string;

    while (*cursor) {
        char const* next = strstr(cursor+1, "\\c");
        if (!next) {
            next = cursor + strlen(cursor);
        }

        if (strnicmp(cursor, "\\c", 2) == 0) {
            cursor += 2;    // skip past marker
            if (strnicmp(cursor, "x", 1) == 0) {
                r = color.r * 255;
                g = color.g * 255;
                b = color.b * 255;
                cursor++;
            } else {
                sscanf(cursor, "%02x%02x%02x", &r, &g, &b);
                cursor += 6;    // skip past color
            }
        }

        glColor4ub(r, g, b, a);
        glRasterPos2f(position.x + xoffs, position.y);
        glCallLists(next - cursor, GL_UNSIGNED_BYTE, cursor);

        while (cursor < next) {
            xoffs += _char_width[*cursor++];
        }
    }
}

//------------------------------------------------------------------------------
vec2 font::size(char const* string) const
{
    vec2 size(0, _size);
    char const* cursor = string;

    while (*cursor) {
        char const* next = strstr(cursor+1, "\\c");
        if (!next) {
            next = cursor + strlen(cursor);
        }

        if (strnicmp(cursor, "\\c", 2) == 0) {
            cursor += 2;    // skip past marker
            if (strnicmp(cursor, "x", 1) == 0) {
                cursor++;
            } else {
                cursor += 6;    // skip past color
            }
        }

        while (cursor < next) {
            size.x += _char_width[*cursor++];
        }
    }

    return size;
}

} // namespace render
