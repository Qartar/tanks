//  r_image.cpp
//

#include "precompiled.h"
#pragma hdrstop

#ifndef GL_VERSION_1_2
#define GL_BGR                            0x80E0
#endif // GL_VERSION_1_2

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
render::image const* system::load_image(string::view name)
{
    _images.push_back(std::make_unique<render::image>(name));
    return _images.back().get();
}

//------------------------------------------------------------------------------
void system::draw_image(render::image const* img, vec2 org, vec2 sz, color4 color)
{
    if (img == nullptr) {
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, img->texnum());

    glColor4fv(color);

    glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(org.x, org.y);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(org.x + sz.x, org.y);

        glTexCoord2f(0.0f, 1.0f );
        glVertex2f(org.x, org.y + sz.y);

        glTexCoord2f(1.0f, 1.0f );
        glVertex2f(org.x + sz.x, org.y + sz.y);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

//------------------------------------------------------------------------------
image::image(string::view name)
    : _texnum(0)
    , _width(0)
    , _height(0)
{
    HBITMAP bitmap = NULL;

    if ((bitmap = load_resource(name))) {
        _name = string::buffer(va("<resource#%d>", (ULONG_PTR)name.begin()));
    } else if ((bitmap = load_file(name))) {
        _name = string::buffer(name);
    } else {
        _name = string::buffer("<default>");
    }

    upload(bitmap);

    if (bitmap) {
        DeleteObject(bitmap);
    }
}

//------------------------------------------------------------------------------
image::~image()
{
    if (_texnum) {
        glDeleteTextures(1, &_texnum);
    }
}

//------------------------------------------------------------------------------
HBITMAP image::load_resource(string::view name) const
{
    UINT flags = LR_CREATEDIBSECTION;

    return (HBITMAP )LoadImageA(
        g_Application->hinstance(),     // hinst
        name.begin(),                   // name
        IMAGE_BITMAP,                   // type
        0,                              // cx
        0,                              // cy
        flags                           // fuLoad
    );
}

//------------------------------------------------------------------------------
HBITMAP image::load_file(string::view name) const
{
    UINT flags = LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE;

    return (HBITMAP )LoadImageA(
        NULL,                           // hinst
        name.c_str(),                   // name
        IMAGE_BITMAP,                   // type
        0,                              // cx
        0,                              // cy
        flags                           // fuLoad
    );
}

//------------------------------------------------------------------------------
bool image::upload(HBITMAP bitmap)
{
    if (!bitmap) {
        return false;
    }

    BITMAP bm;
    
    if (!GetObjectA(bitmap, sizeof(bm), &bm)) {
        return false;
    }

    std::vector<uint8_t> buffer(bm.bmWidthBytes * bm.bmHeight);

    if (!GetBitmapBits(bitmap, narrow_cast<LONG>(buffer.size()), buffer.data())) {
        return false;
    }

    _width = bm.bmWidth;
    _height = bm.bmHeight;

    glGenTextures(1, &_texnum);
    glBindTexture(GL_TEXTURE_2D, _texnum);

    glTexImage2D(
        GL_TEXTURE_2D,      // target
        0,                  // level
        GL_RGB,             // internalformat
        bm.bmWidth,         // width
        bm.bmHeight,        // height
        0,                  // border
        GL_BGR,             // format
        GL_UNSIGNED_BYTE,   // type
        buffer.data()       // pixels
    );

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return true;
}

} // namespace render
