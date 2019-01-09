// g_usercmd.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "g_usercmd.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void usercmdgen::reset(bool unbind_all/* = false*/)
 {
    _button_state = 0;
    _gamepad_state = {};
    if (unbind_all) {
        _bindings.clear();
    }
}

//------------------------------------------------------------------------------
void usercmdgen::bind(int key, usercmdgen::button button)
{
    _bindings[key] = button;
}

//------------------------------------------------------------------------------
void usercmdgen::unbind(int key)
{
    _bindings.erase(key);
}

//------------------------------------------------------------------------------
bool usercmdgen::key_event(int key, bool down)
{
    auto it = _bindings.find(key);
    if (it != _bindings.end()) {
        if (down) {
            _button_state |= (1 << static_cast<int>(it->second));
        } else {
            _button_state &= ~(1 << static_cast<int>(it->second));
        }
        return true;
    } else {
        return false;
    }
}

//------------------------------------------------------------------------------
void usercmdgen::gamepad_event(gamepad const& pad)
{
    _gamepad_state = pad;
}

//------------------------------------------------------------------------------
int usercmdgen::state(usercmdgen::button button) const
{
    return !!(_button_state & (1 << static_cast<int>(button)));
}

//------------------------------------------------------------------------------
usercmd usercmdgen::generate() const
{
    usercmd cmd{};

    cmd.move = _gamepad_state.thumbstick[gamepad::left];
    cmd.look = _gamepad_state.thumbstick[gamepad::right];

    cmd.move.x += static_cast<float>(state(button::right) - state(button::left));
    cmd.move.y += static_cast<float>(state(button::forward) - state(button::back));
    cmd.look.x += static_cast<float>(state(button::turret_right) - state(button::turret_left));

    if (cmd.move.length_sqr() > 1.f) {
        cmd.move.normalize_self();
    }

    if (cmd.look.length_sqr() > 1.f) {
        cmd.look.normalize_self();
    }

    if (state(button::fire) || _gamepad_state.trigger[gamepad::right] > 0.1f) {
        cmd.action = usercmd::action::attack;
    } else {
        cmd.action = usercmd::action::none;
    }

    return cmd;
}

} // namespace game
