// g_usercmd.h
//

#pragma once

#include "oed_types.h"

#include <cstdint>
#include <map>

namespace game {

//------------------------------------------------------------------------------
struct usercmd
{
    enum class action
    {
        none,
        attack,
    };

    vec2 move; //!< forward/back, left/right
    vec2 look; //!< turret left/right, _
    action action;
};

//------------------------------------------------------------------------------
class usercmdgen
{
public:
    enum class button
    {
        forward,
        back,
        left,
        right,
        turret_left,
        turret_right,
        fire,
    };

    usercmdgen()
        : _button_state(0)
    {}

    void reset(bool unbind_all = false);
    void bind(int key, button button);
    void unbind(int key);
    template<std::size_t Size> void bind(std::pair<int, button> const (&bindings)[Size]);

    bool key_event(int key, bool down);

    int state(button button) const;
    usercmd generate() const;

protected:
    std::map<int, button> _bindings;
    uint32_t _button_state;
};

//------------------------------------------------------------------------------
template<size_t Size> void usercmdgen::bind(std::pair<int, button> const (&bindings)[Size])
{
    for (std::size_t ii = 0; ii < Size; ++ii) {
        bind(bindings[ii].first, bindings[ii].second);
    }
}

} // namespace game
