/*
===============================================================================

Name    :   g_menu.h

Purpose :   handles menu functions

Date    :   10/20/2004

===============================================================================
*/

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace render {
class system;
}

namespace menu {

class window;

const vec4 colors[] =
{
    vec4(0.000,0.000,0.000,1),
    vec4(0.125,0.125,0.125,1),
    vec4(0.250,0.250,0.250,1),
    vec4(0.375,0.375,0.375,1),
    vec4(0.500,0.500,0.500,1),
    vec4(0.625,0.625,0.625,1),
    vec4(0.750,0.750,0.750,1),
    vec4(0.875,0.875,0.875,1),
    vec4(1.000,1.000,1.000,1),
};

//------------------------------------------------------------------------------
class rectangle
{
public:
    rectangle() {}
    rectangle(vec2 center, vec2 size)
        : _position(center - size * 0.5f)
        , _size(size)
    {}

    //! Coordinate of top-left corner
    vec2 position() const { return _position; }

    //! Coordinate of center
    vec2 center() const { return _position + _size * 0.5f; }

    //! Rectangle size
    vec2 size() const { return _size; }

    //! Returns an equivalent rectangle translated by `v`
    rectangle operator+(vec2 v) const { return rectangle(center() + v, _size); }

    //! Returns an equivalent rectangle translated by the negative of `v`
    rectangle operator-(vec2 v) const { return rectangle(center() - v, _size); }

    //! Returns true if `point` is contained inside rectangle
    bool contains(vec2 point) const {
        return point.x > _position.x
            && point.x < _position.x + _size.x
            && point.y > _position.y
            && point.y < _position.y + _size.y;
    }

protected:
    vec2 _position;
    vec2 _size;
};

//------------------------------------------------------------------------------
class button
{
public:
    button(char const* text, vec2 center, vec2 size)
        : _text(text)
        , _rectangle(center, size)
        , _down(false)
    {}

    template<typename Tfunc>
    button(char const* text, vec2 center, vec2 size, Tfunc&& func)
        : button(text, center, size)
    {
        _func = std::move(func);
    }

    virtual ~button() {}

    virtual bool click(vec2 cursor_pos, bool down);
    virtual void draw(render::system* renderer, vec2 cursor_pos) const;

protected:
    std::string _text;
    menu::rectangle _rectangle;
    std::function<void()> _func;
    bool _down;

protected:
    enum text_flags {
        align_default   = 0,

        // horizontal alignment
        halign_center   = align_default,
        halign_left     = BIT(0),
        halign_right    = BIT(1),

        // vertical alignment
        valign_center   = align_default,
        valign_top      = BIT(2),
        valign_bottom   = BIT(3),
    };

    void draw_rectangle(render::system* renderer, menu::rectangle const& rect, vec4 color) const;
    void draw_rectangle(render::system* renderer, menu::rectangle const& rect, vec4 color, vec4 border_color) const;
    void draw_text(render::system* renderer, menu::rectangle const& rect, std::string const& text, vec4 color, int flags = align_default, float margin = 4.0f) const;
};

//------------------------------------------------------------------------------
class server_button : public button
{
public:
    server_button(vec2 position, vec2 size, char const* name_ptr, float const* ping_ptr, std::function<void()>&& op_click);

    virtual bool click(vec2 cursor_pos, bool down) override;
    virtual void draw(render::system* renderer, vec2 cursor_pos) const override;

protected:
    char const* _name_ptr;
    float const* _ping_ptr;

    menu::rectangle _join_rectangle;
    menu::rectangle _text_rectangle;
};

//------------------------------------------------------------------------------
class host_button : public button
{
public:
    host_button(vec2 position, vec2 size, std::function<void()>&& op_click);

    virtual bool click(vec2 cursor_pos, bool down) override;
    virtual void draw(render::system* renderer, vec2 cursor_pos) const override;

protected:
    menu::rectangle _create_rectangle;
    menu::rectangle _text_rectangle;

    bool _create_down;
    bool _text_down;
};

//------------------------------------------------------------------------------
class conditional_button : public button
{
public:
    conditional_button(char const* text, vec2 position, vec2 size, bool *condition_ptr, std::function<void()>&& op_click);

    virtual bool click(vec2 cursor_pos, bool down) override;
    virtual void draw(render::system* renderer, vec2 cursor_pos) const override;

protected:
    bool* _condition_ptr;
};

//------------------------------------------------------------------------------
class client_button : public button
{
public:
    client_button(char const* text, vec2 position, vec2 size, vec4 *pColor);

    virtual bool click(vec2 cursor_pos, bool down) override;
    virtual void draw(render::system* renderer, vec2 cursor_pos) const override;

protected:
    vec4* _color_ptr;
    int _color_index;

    bool _text_down;

    menu::rectangle _text_rectangle;
};

//------------------------------------------------------------------------------
class submenu_button : public button
{
public:
    submenu_button (char const* text, vec2 position, vec2 size, menu::window *pParent, menu::window *pMenu);

    virtual void draw(render::system* renderer, vec2 cursor_pos) const override;

    void deactivate() { _active = false; }

protected:
    menu::window* _parent;
    menu::window* _menu;
    bool _active;
};

/*
===========================================================

Name    :   cMenu

Purpose :   controller class for a menu set

===========================================================
*/

class window
{
public:
    window ()
        : _active_button(nullptr)
        , _active_menu(nullptr)
    {}

    void init ();
    void shutdown ();

    template<typename T, typename... Args>
    void add_button(Args&& ...args);

    virtual void draw(render::system* renderer, vec2 cursor_pos) const;
    virtual void click(vec2 cursor_pos, bool down);

    bool activate(submenu_button* button, menu::window* submenu);

protected:
    std::vector<std::unique_ptr<menu::button>> _buttons;
    std::vector<std::unique_ptr<menu::window>> _submenus;

    submenu_button* _active_button;
    menu::window* _active_menu;
};

template<typename T, typename... Args>
void window::add_button(Args&& ...args)
{
    _buttons.push_back(std::make_unique<T>(args...));
}

} // namespace menu
