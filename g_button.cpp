// g_button.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "keys.h"

////////////////////////////////////////////////////////////////////////////////
namespace menu {

//------------------------------------------------------------------------------
bool button::key_event(int key, bool down)
{
    if (key == K_MOUSE1) {
        if (_over && down) {
            _down = true;
            return true;
        } else if (!down) {
            if (_down && _over && _func) {
                _func();
            }
            _down = false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
bool button::cursor_event(vec2 position)
{
    _over = _rectangle.contains(position);
    return false;
}

//------------------------------------------------------------------------------
void button::draw(render::system* renderer) const
{
    int border_color = _over ? 6 : 4;
    int button_color = _down ? 3 : 5;
    int text_color = button_color + 2;

    draw_rectangle(renderer, _rectangle, menu::colors[button_color], menu::colors[border_color]);
    draw_text(renderer, _rectangle, _text, menu::colors[text_color]);
}

//------------------------------------------------------------------------------
void button::draw_rectangle(render::system* renderer, menu::rectangle const& rect, color4 color) const
{
    renderer->draw_box(rect.size(), rect.center(), color);
}

//------------------------------------------------------------------------------
void button::draw_rectangle(render::system* renderer, menu::rectangle const& rect, color4 color, color4 border_color) const
{
    renderer->draw_box(rect.size(), rect.center(), border_color);
    renderer->draw_box(rect.size() - vec2(2, 2), rect.center(), color);
}

//------------------------------------------------------------------------------
void button::draw_text(render::system* renderer, menu::rectangle const& rect, std::string const& text, color4 color, int flags, float margin) const
{
    vec2 position = rect.center();
    vec2 size = renderer->string_size(text.c_str());

    if (flags & halign_left) {
        position.x -= rect.size().x * 0.5f - margin;
    } else if (flags & halign_right) {
        position.x += rect.size().x * 0.5f - size.x - margin;
    } else {
        position.x -= size.x * 0.5f;
    }

    if (flags & valign_top) {
        position.y -= rect.size().y * 0.5f - size.y - margin;
    } else if (flags & valign_bottom) {
        position.y += rect.size().y * 0.5f - margin;
    } else {
        position.y += size.y * 0.5f;
    }

    position.x = std::floor(position.x + 0.5f);
    position.y = std::floor(position.y + 0.5f);

    renderer->draw_string(text.c_str(), position, color);
}

////////////////////////////////////////////////////////////////////////////////
conditional_button::conditional_button(char const* text, vec2 position, vec2 size, bool* condition_ptr, std::function<void()>&& op_click)
    : button(text, position, size, std::move(op_click))
    , _condition_ptr(condition_ptr)
{}

//------------------------------------------------------------------------------
bool conditional_button::key_event(int key, bool down)
{
    if (_condition_ptr && !*_condition_ptr) {
        _down = false;
        return false;
    }

    return button::key_event(key, down);
}

//------------------------------------------------------------------------------
void conditional_button::draw(render::system* renderer) const
{
    int border_color = _over ? 6 : 4;
    int button_color = _down ? 3 : 5;
    int text_color = button_color + 2;

    if (_condition_ptr && !*_condition_ptr) {
        border_color = 1;
        button_color = 2;
        text_color = 3;
    }

    draw_rectangle(renderer, _rectangle, menu::colors[button_color], menu::colors[border_color]);
    draw_text(renderer, _rectangle, _text, menu::colors[text_color]);
}


////////////////////////////////////////////////////////////////////////////////
submenu_button::submenu_button(char const* text, vec2 position, vec2 size, menu::window* parent, menu::window* menu)
    : button(text, position, size, [this](){ _active = _parent->activate(this, _menu); })
    , _parent(parent)
    , _menu(menu)
    , _active(false)
{}

//------------------------------------------------------------------------------
void submenu_button::draw(render::system* renderer) const
{
    int border_color = _over ? 6 : 4;
    int button_color = (_down || _active) ? 3 : 5;
    int text_color = button_color + 2;

    if (_menu == nullptr) {
        border_color = 1;
        button_color = 2;
        text_color = 3;
    }

    draw_rectangle(renderer, _rectangle, menu::colors[button_color], menu::colors[border_color]);
    draw_text(renderer, _rectangle, _text, menu::colors[text_color]);
}

////////////////////////////////////////////////////////////////////////////////
client_button::client_button(char const* text, vec2 position, vec2 size, color3* color_ptr)
    : button(text, position, size)
    , _text_rectangle(position + vec2(0, size.y / 2.0f - 12.0f), vec2(size.x - 16.0f, 14.0f))
    , _color_ptr(color_ptr)
    , _color_index(0)
    , _text_down(false)
{}

//------------------------------------------------------------------------------
bool client_button::key_event(int key, bool down)
{
    if (key == K_MOUSE1) {
        if (_text_over && down) {
            _text_down = true;
            return true;
        } else if (!down) {
            if (_text_down && _text_over) {
                g_Game->_client_button_down ^= 1;
            }
            _text_down = false;
        }

        if (_over && down) {
            _down = true;
            return true;
        } else if (!down) {
            if (_down && _over) {
                _color_index = (_color_index+1) % game::num_player_colors;
                *_color_ptr = game::player_colors[_color_index];
            }
            _down = false;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
bool client_button::cursor_event(vec2 position)
{
    _text_over = _text_rectangle.contains(position);
    _over = !_text_over && _rectangle.contains(position);
    return false;
}

//------------------------------------------------------------------------------
void client_button::draw(render::system* renderer) const
{
    int text_border_color = (g_Game->_client_button_down || _text_over) ? 6 : 4;
    int text_button_color = (g_Game->_client_button_down || _text_down) ? 3 : 5;

    int border_color = _over ? 6 : 4;
    int button_color = _down ? 3 : 5;
    int text_color = button_color + 2;

    draw_rectangle(renderer, _rectangle, menu::colors[button_color], menu::colors[border_color]);
    draw_text(renderer, _rectangle, _text, menu::colors[text_color], valign_top, 8.0f);

    draw_rectangle(renderer, _text_rectangle, menu::colors[text_button_color], menu::colors[text_border_color]);
    draw_text(renderer, _text_rectangle, g_Game->cls.name, menu::colors[7], valign_bottom|halign_left);

    tank_body_model.draw(_rectangle.center(), 0, color4(*_color_ptr));
    tank_turret_model.draw(_rectangle.center(), 0, color4(*_color_ptr));
}

////////////////////////////////////////////////////////////////////////////////
server_button::server_button(vec2 position, vec2 size, char const* name_ptr, float const* ping_ptr, std::function<void()>&& op_click)
    : button("", position, size, std::move(op_click))
    , _join_rectangle(position + vec2(size.x * 0.5f - 18.0f, 0), vec2(32, size.y - 4.0f))
    , _text_rectangle(position - vec2(17, 0), size - vec2(38, 4))
    , _name_ptr(name_ptr)
    , _ping_ptr(ping_ptr)
{}

//------------------------------------------------------------------------------
bool server_button::key_event(int key, bool down)
{
    if (key == K_MOUSE1) {
        if (!_name_ptr || !_name_ptr[0]) {
            _down = false;
            return false;
        }

        if (_over && down) {
            _down = true;
            return true;
        } else if (!down) {
            if (_down && _over) {
                _func();
            }
            _down = false;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void server_button::draw(render::system* renderer) const
{
    draw_rectangle(renderer, _rectangle, menu::colors[3], menu::colors[4]);
    draw_rectangle(renderer, _text_rectangle, menu::colors[0], menu::colors[2]);

    // join button

    if (_name_ptr && _name_ptr[0]) {
        draw_text(renderer, _text_rectangle, _name_ptr, menu::colors[7], halign_left);
        draw_text(renderer, _text_rectangle, va("%i", (int)(*_ping_ptr)), menu::colors[7], halign_right);

        int border_color = _over ? 6 : 4;
        int button_color = _down ? 3 : 5;
        int text_color = 7;

        draw_rectangle(renderer, _join_rectangle, menu::colors[button_color], menu::colors[border_color]);
        draw_text(renderer, _join_rectangle, "Join", menu::colors[text_color]);
    } else {
        draw_rectangle(renderer, _join_rectangle, menu::colors[4], menu::colors[2]);
        draw_text(renderer, _join_rectangle, "Join", menu::colors[2]);
    }
}

////////////////////////////////////////////////////////////////////////////////
host_button::host_button(vec2 position, vec2 size, std::function<void()>&& op_click)
    : button("Host", position, size, std::move(op_click))
    , _create_rectangle(position + vec2(size.x * 0.5f - 18.0f, 0), vec2(32, size.y - 4.0f))
    , _text_rectangle(position - vec2(17, 0), size - vec2(38, 4))
    , _create_down(false)
    , _text_down(false)
    , _create_over(false)
    , _text_over(false)
{}

//------------------------------------------------------------------------------
bool host_button::key_event(int key, bool down)
{
    if (key == K_MOUSE1) {
        if (_create_over && down) {
            _create_down = true;
            return true;
        } else if (!down) {
            if (_create_down && _create_over) {
                _func();
            }
            _create_down = false;
        }

        if (_text_over && down) {
            _text_down = true;
            return true;
        } else if (!down) {
            if (_text_down && _text_over) {
                g_Game->_server_button_down ^= 1;
            }
            _text_down = false;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
bool host_button::cursor_event(vec2 position)
{
    _text_over = _text_rectangle.contains(position);
    _create_over = _create_rectangle.contains(position);
    return false;
}

//------------------------------------------------------------------------------
void host_button::draw(render::system* renderer) const
{
    int text_button_color = g_Game->_server_button_down ? 5 : 3;

    draw_rectangle(renderer, _rectangle, menu::colors[3], menu::colors[4]);
    draw_rectangle(renderer, _text_rectangle, menu::colors[text_button_color]);
    draw_text(renderer, _text_rectangle, g_Game->svs.name, menu::colors[7], halign_left);

    int border_color = _create_over ? 6 : 4;
    int button_color = _down ? 3 : 5;
    int text_color = 7;

    draw_rectangle(renderer, _create_rectangle, menu::colors[button_color], menu::colors[border_color]);
    draw_text(renderer, _create_rectangle, "Create", menu::colors[text_color]);
}

} // namespace menu
