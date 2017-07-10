// g_usercmd.cpp
//

#include "g_usercmd.h"

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
void usercmdgen::reset(bool unbind_all/* = false*/)
 {
    _button_state = 0;
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
int usercmdgen::state(usercmdgen::button button) const
{
    return !!(_button_state & (1 << static_cast<int>(button)));
}

//------------------------------------------------------------------------------
usercmd usercmdgen::generate() const
{
    usercmd cmd{};

    cmd.move.x = static_cast<float>(state(button::forward) - state(button::back));
    cmd.move.y = static_cast<float>(state(button::left) - state(button::right));
    cmd.look.x = static_cast<float>(state(button::turret_left) - state(button::turret_right));
    cmd.action = state(button::fire) ? usercmd::action::attack : usercmd::action::none;

    return cmd;
}

} // namespace game
