// g_usercmd.h
//

#pragma once

#include "cm_vector.h"

#include <cstdint>
#include <map>

////////////////////////////////////////////////////////////////////////////////
namespace game {

//------------------------------------------------------------------------------
struct gamepad
{
    enum side
    {
        left = 0,
        right = 1,
    };

    //! left and right thumbstick state, normalized to unit length
    vec2 thumbstick[2];
    //! left and right trigger state, normalized to [0,1]
    float trigger[2];
};

//------------------------------------------------------------------------------
struct usercmd
{
    enum class action
    {
        none,
        attack,
    };

    vec2 move; //!< right/left, forward/back
    vec2 look; //!< turret right/left, _
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
        , _gamepad_state{}
    {}

    void reset(bool unbind_all = false);
    void bind(int key, button button);
    void unbind(int key);
    template<std::size_t Size> void bind(std::pair<int, button> const (&bindings)[Size]);

    bool key_event(int key, bool down);
    void gamepad_event(gamepad const& pad);

    int state(button button) const;
    usercmd generate() const;

protected:
    std::map<int, button> _bindings;
    uint32_t _button_state;
    gamepad _gamepad_state;
};

//------------------------------------------------------------------------------
template<size_t Size> void usercmdgen::bind(std::pair<int, button> const (&bindings)[Size])
{
    for (std::size_t ii = 0; ii < Size; ++ii) {
        bind(bindings[ii].first, bindings[ii].second);
    }
}

} // namespace game
