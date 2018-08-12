// p_world.cpp
//

#include "p_world.h"
#include "p_collide.h"
#include "p_rigidbody.h"
#include "p_trace.h"
#include <algorithm>
#include <cassert>
#include <queue>

////////////////////////////////////////////////////////////////////////////////
namespace physics {

//------------------------------------------------------------------------------
world::world(filter_callback_type filter_callback, collision_callback_type collision_callback)
    : _filter_callback(filter_callback)
    , _collision_callback(collision_callback)
{
}

//------------------------------------------------------------------------------
void world::add_body(physics::rigid_body* body)
{
    assert(std::find(_bodies.begin(), _bodies.end(), body) == _bodies.end());
    _bodies.push_back(body);
}

//------------------------------------------------------------------------------
void world::remove_body(physics::rigid_body* body)
{
    assert(std::find(_bodies.begin(), _bodies.end(), body) != _bodies.end());
    _bodies.erase(std::find(_bodies.begin(), _bodies.end(), body));
}

//------------------------------------------------------------------------------
void world::step(float delta_time)
{
    struct collision {
        std::size_t body_b;
        physics::trace tr;

        bool operator<(collision const& other) const {
            return tr.get_fraction() > other.tr.get_fraction(); // sort ascending
        }
    };

    for (std::size_t ii = 0; ii < _bodies.size(); ++ii) {
        std::priority_queue<collision> collisions;

        // check all pairs, including permutations
        for (std::size_t jj = 0; jj < _bodies.size(); ++jj) {
            // cannot collide with itself
            if (ii == jj) {
                continue;
            }

            // check collision filter
            if (_filter_callback && !_filter_callback(_bodies[ii], _bodies[jj])) {
                continue;
            }

            // check collision
            physics::trace tr(_bodies[ii], _bodies[jj], delta_time);
            if (tr.get_fraction() == 1.f) {
                continue;
            }

            collisions.push({jj, tr});
        }

        // use the earliest collision
        while (collisions.size()) {
            collision c = collisions.top();
            std::size_t jj = c.body_b;
            collisions.pop();

            if (collisions.size()) {
                assert(collisions.top().tr.get_fraction() >= c.tr.get_fraction());
            }

            // check collision callback
            if (_collision_callback && !_collision_callback(_bodies[ii], _bodies[jj], c.tr.get_contact())) {
                continue;
            }

            // collision response
            _bodies[ii]->apply_impulse(
                -c.tr.get_contact().impulse,
                c.tr.get_contact().point);

            _bodies[jj]->apply_impulse(
                c.tr.get_contact().impulse,
                c.tr.get_contact().point);

            break;
        }
    }

    // move

    for (std::size_t ii = 0; ii < _bodies.size(); ++ii) {
        _bodies[ii]->set_position(_bodies[ii]->get_position() + _bodies[ii]->get_linear_velocity() * delta_time);
        _bodies[ii]->set_rotation(_bodies[ii]->get_rotation() + _bodies[ii]->get_angular_velocity() * delta_time);
    }
}

} // namespace physics
