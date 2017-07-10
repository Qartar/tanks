//  r_image.cpp
//

#include "local.h"
#pragma hdrstop

#ifndef GL_VERSION_1_2
#define GL_BGR                            0x80E0
#endif // GL_VERSION_1_2

////////////////////////////////////////////////////////////////////////////////
namespace render {

//------------------------------------------------------------------------------
render::image const* system::load_image(const char *name)
{
    _images.push_back(std::make_unique<render::image>(name));
    return _images.back().get();
}

//------------------------------------------------------------------------------
void system::draw_image(render::image const* img, vec2 org, vec2 sz, vec4 color)
{
    if (img == nullptr) {
        return;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, img->texnum());

    glColor4fv(color.v);

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
image::image(char const* name)
    : _texnum(0)
    , _width(0)
    , _height(0)
{
    HBITMAP bitmap = NULL;

    if ((bitmap = load_resource(name))) {
        _name = va("<resource#%d>", (ULONG_PTR)name);
    } else if ((bitmap = load_file(name))) {
        _name = name;
    } else {
        _name = "<default>";
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
HBITMAP image::load_resource(char const* name) const
{
    UINT flags = LR_CREATEDIBSECTION;

    return (HBITMAP )LoadImageA(
        g_Application->get_hInstance(), // hinst
        name,                           // name
        IMAGE_BITMAP,                   // type
        0,                              // cx
        0,                              // cy
        flags                           // fuLoad
    );
}

//------------------------------------------------------------------------------
HBITMAP image::load_file(char const* name) const
{
    UINT flags = LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE;

    return (HBITMAP )LoadImageA(
        NULL,                           // hinst
        name,                           // name
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

    if (!GetBitmapBits(bitmap, buffer.size(), buffer.data())) {
        return false;
    }

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
