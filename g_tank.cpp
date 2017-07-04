/*
===============================================================================

Name    :   g_tank.cpp

Purpose :   tanks!

===============================================================================
*/

#include "local.h"
#pragma hdrstop

#include "keys.h"

namespace game {

physics::material tank::_material(0.5f, 1.0f, 5.0f);
physics::box_shape tank::_shape(vec2(24, 16));

//------------------------------------------------------------------------------
tank::tank ()
    : object(object_type::tank)
    , _turret_model(nullptr)
    , _turret_rotation(0)
    , _turret_velocity(0)
    , _old_turret_rotation(0)
    , _track_speed(0)
    , _damage(0)
    , _player_index(0)
    , _dead_time(0)
    , _fire_time(0)
    , _keys{0}
    , _channels{0}
    , _client(nullptr)
{
    _rigid_body = physics::rigid_body(&_shape, &_material, 1.0f);

    for (auto& chan : _channels) {
        if (chan == nullptr) {
            chan = pSound->allocChan();
        }
    }
}

//------------------------------------------------------------------------------
tank::~tank()
{
    for (auto& chan : _channels) {
        if (chan) {
            chan->stopSound();
            pSound->freeChan(chan);
        }
    }
}

//------------------------------------------------------------------------------
void tank::draw() const
{
    float   flLerp = (g_Game->m_flTime - (g_Game->m_nFramenum-1) * FRAMEMSEC) / FRAMEMSEC;
    float   flTime = clamp(((g_Game->m_flTime - _fire_time) / 150.f) * _client->refire_mod,0,20);
    float   flHealth = clamp((1-_damage)*20,0,20);

    if ( flLerp > 1.0f ) {
        flLerp = 1.0f;
    } else if ( flLerp < 0.0f ) {
        flLerp = 0.0f;
    }

    vec2    pos = get_position( flLerp );
    float   angle = get_rotation( flLerp );
    float   tangle = get_turret_rotation( flLerp );

    vec4    vHealth;
    vec4    vTime;

    vHealth.r = ( flHealth < 10.0f ? 1.0f : (10.0f - (flHealth - 10.0f)) / 10.0f );
    vHealth.g = ( flHealth > 10.0f ? 1.0f : (flHealth / 10.0f) );
    vHealth.b = 0.0f;
    vHealth.a = 1.0f;

    vTime.r = 1.0f;
    vTime.g = flTime/20.0f;
    vTime.b = ( flTime == 20.0f ? 1.0f : 0.0f );
    vTime.a = 1.0f;

    if (_damage >= 1.0f)
    {
        _model->draw(pos, angle, vec4(0.3f,0.3f,0.3f,1));
        _turret_model->draw(pos, tangle, vec4(0.3f,0.3f,0.3f,1));
        return;
    }

    // status bars

    g_Render->DrawBox( vec2(20,2), pos + vec2(0,24), 0, vec4(0.5,0.5,0.5,1) );
    g_Render->DrawBox( vec2(flTime,2), pos + vec2(0,24), 0, vTime );
    g_Render->DrawBox( vec2(20,2), pos + vec2(0,22), 0, vec4(0.5,0.5,0.5,1) );
    g_Render->DrawBox( vec2(flHealth,2), pos + vec2(0,22), 0, vHealth );

    // actual body

    _model->draw(pos, angle, _color);
    _turret_model->draw(pos, tangle, _color);
}

//------------------------------------------------------------------------------
void tank::touch(object *other, float impulse)
{
    if (!other) {
        return;
    }

    if (other->_type != object_type::tank) {
        return;
    }

    float base_damage = (max(0, impulse - 5.0f) / 5.0f) * FRAMETIME;
    tank* other_tank = static_cast<tank*>(other);

    this->collide(other_tank, base_damage);
    other_tank->collide(this, base_damage);
}

//------------------------------------------------------------------------------
void tank::collide(tank* other, float base_damage)
{
    if (other->_damage < 1.0f) {
        other->_damage += base_damage / other->_client->armor_mod;

        if (other->_damage >= 1.0f)
        {
            g_Game->AddScore(_player_index, 1);
            other->_dead_time = g_Game->m_flTime;
            if (g_Game->bAutoRestart && !g_Game->m_bMultiplayer)
                g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

            g_Game->m_WriteMessage( va("%s got a little too cozy with %s.", other->player_name(), player_name() ) );
        }
    }
}

#define HACK_TIME       1000.0f

//------------------------------------------------------------------------------
void tank::think()
{
    float   flForward;
    float   flLeft;
    float   flTLeft;
    float   flVel;

    _old_turret_rotation = _turret_rotation;

    if ( (_damage >= 1.0f) && g_Game->m_bMultiserver && (_dead_time+RESTART_TIME+HACK_TIME <= g_Game->m_flTime) )
    {
        // respawn

        int nWidth = g_World->_mins.x - g_World->_mins.x;
        int nHeight = g_World->_maxs.y - g_World->_maxs.y;

        nWidth -= SPAWN_BUFFER*2;
        nHeight -= SPAWN_BUFFER*2;

        _dead_time = 0.0f;

        set_position(vec2(g_World->_mins.x + nWidth*frand()+SPAWN_BUFFER,
                          g_World->_mins.y + nHeight*frand()+SPAWN_BUFFER));
        set_rotation(frand()*2.0f*M_PI);
        _turret_rotation = get_rotation();

        set_linear_velocity(vec2(0,0));
        set_angular_velocity(0);
        _turret_velocity = 0;

        _damage = 0.0f;
    }

    flForward = _keys[KEY_FORWARD] - _keys[KEY_BACK];
    flLeft = _keys[KEY_LEFT] - _keys[KEY_RIGHT];
    flTLeft = _keys[KEY_TLEFT] - _keys[KEY_TRIGHT];

    vec2 forward = rot(vec2(1,0), get_rotation());
    flVel = forward.dot(get_linear_velocity());

    vec2 track_velocity = rot(vec2(_track_speed,0), get_rotation());
    vec2 delta_velocity = track_velocity - get_linear_velocity();
    vec2 friction_impulse = delta_velocity * _rigid_body.get_mass() * _rigid_body.get_material()->sliding_friction() * FRAMETIME;

    _rigid_body.apply_impulse(friction_impulse);

    if (_damage >= 1.0f)
    {
        vec2 vVel = vec2(flVel * 0.98 * (1-FRAMETIME),0);

        set_linear_velocity(rot(vVel,get_rotation()));
        set_angular_velocity(get_angular_velocity() * 0.9f);
        _turret_velocity *= 0.9f;

        // extra explosion
        if (_dead_time && (g_Game->m_flTime - _dead_time > 650) && (g_Game->m_flTime - _dead_time < 650+HACK_TIME/2))
        {
            g_World->add_sound( sound_index[TANK_EXPLODE].name );
            g_World->add_effect( get_position(), effect_type::explosion );
            _dead_time -= HACK_TIME;    // dont do it again
        }
    }
    else
    {
        float new_speed = _track_speed * 0.9 * (1 - FRAMETIME) + flForward * 192 * _client->speed_mod * FRAMETIME;
        new_speed = clamp(new_speed, -32 * _client->speed_mod, 48 * _client->speed_mod);

        set_linear_velocity(get_linear_velocity() + forward * (new_speed - _track_speed));
        _track_speed = new_speed;

        set_angular_velocity(deg2rad(-flLeft * 90));
        _turret_velocity = deg2rad(-flTLeft * 90);
    }

    // update position here because Move doesn't
    _turret_rotation += _turret_velocity * FRAMETIME;

    if ((_fire_time + 2500/_client->refire_mod) > g_Game->m_flTime)  // just fired
    {
        vec2    vOrg(21,0);
        float   flPower;

        flPower = 1.5 - (g_Game->m_flTime - _fire_time)/1000.0f;
        flPower = clamp( flPower, 0.5, 1.5 );

        g_World->add_smoke_effect(
            get_position() + rot(vOrg,_turret_rotation),
            rot(vOrg,_turret_rotation) * flPower * flPower * flPower * 2,
            flPower * flPower * 4 );
    }
    else if ((_fire_time + 3000/_client->refire_mod) < g_Game->m_flTime) // can fire
    {
        // check for firing
        if (_keys[KEY_FIRE] && _damage < 1.0f)
        {
            vec2    vOrg(21,0);

            g_World->add_smoke_effect(
                get_position() + rot(vOrg,_turret_rotation),
                rot(vOrg,_turret_rotation) * 16,
                64 );

            _fire_time = g_Game->m_flTime;

            projectile* bullet = g_World->spawn<projectile>(this, _client->damage_mod);

            bullet->set_position(get_position() + rot(vOrg,_turret_rotation));
            bullet->set_linear_velocity(rot(vec2(1,0),_turret_rotation) * 20 * 96);

            g_World->add_sound( sound_index[TANK_FIRE].name );
        }
    }

    update_sound( );
}

//------------------------------------------------------------------------------
float tank::get_turret_rotation(float lerp) const
{
    return _old_turret_rotation + (_turret_rotation - _old_turret_rotation) * lerp;
}

//------------------------------------------------------------------------------
void tank::update_keys(int key, bool is_down)
{
    switch ( key )
    {
    case 'w':
    case 'W':
    case K_UPARROW:
        _keys[KEY_FORWARD] = is_down;
        break;

    case 's':
    case 'S':
    case K_DOWNARROW:
        _keys[KEY_BACK] = is_down;
        break;

    case 'a':
    case 'A':
    case K_LEFTARROW:
        _keys[KEY_LEFT] = is_down;
        break;

    case 'd':
    case 'D':
    case K_RIGHTARROW:
        _keys[KEY_RIGHT] = is_down;
        break;

    case 'j':
    case 'J':
    case '4':
    case K_KP_LEFTARROW:
        _keys[KEY_TLEFT] = is_down;
        break;

    case 'l':
    case 'L':
    case '6':
    case K_KP_RIGHTARROW:
        _keys[KEY_TRIGHT] = is_down;
        break;

    case 'k':
    case 'K':
    case '5':
    case K_KP_5:
        _keys[KEY_FIRE] = is_down;
        break;
    }
}

//------------------------------------------------------------------------------
void tank::update_sound()
{
    // engine noise
    if (_channels[0]) {
        if (_damage < 1.0f) {
            if (!_channels[0]->isPlaying()) {
                _channels[0]->playLoop(sound_index[TANK_IDLE].index);
            }
            _channels[0]->setVolume(1.0f);
            _channels[0]->setAttenuation(0.0f);
            _channels[0]->setFrequency(1.0f);
        }
        else if (_channels[0]->isPlaying()) {
            _channels[0]->stopSound();
        }
    }

    // tread noise
    if (_channels[1]) {
        if (get_linear_velocity().lengthsq() > 1.0f || fabs(get_angular_velocity()) > deg2rad(1)) {
            if (!_channels[1]->isPlaying()) {
                _channels[1]->playLoop(sound_index[TANK_MOVE].index);
            }
            _channels[1]->setVolume(1.0f);
            _channels[1]->setAttenuation(0.0f);
        }
        else if (_channels[1]->isPlaying()) {
            _channels[1]->stopSound();
        }
    }

    // turret noise
    if (_channels[2]) {
        if (fabs(get_angular_velocity() - _turret_velocity) > deg2rad(1)) {
            if (!_channels[2]->isPlaying()) {
                _channels[2]->playLoop( sound_index[TURRET_MOVE].index );
            }
            _channels[2]->setVolume(1.0f);
            _channels[2]->setAttenuation(0.0f);
        }
        else if (_channels[2]->isPlaying()) {
            _channels[2]->stopSound( );
        }
    }
}

//------------------------------------------------------------------------------
char const* tank::player_name() const
{
    return va("\\c%02x%02x%02x%s\\cx",
              int(_color.r * 255),
              int(_color.g * 255),
              int(_color.b * 255),
              g_Game->svs.clients[_player_index].name);
}

/*
===========================================================

Name    :   projectile

===========================================================
*/

physics::circle_shape projectile::_shape(1.0f);
physics::material projectile::_material(0.5f, 1.0f);

//------------------------------------------------------------------------------
projectile::projectile(tank* owner, float damage)
    : object(object_type::projectile, owner)
    , _damage(damage)
{
    _rigid_body = physics::rigid_body(&_shape, &_material, 1.0f);
}

#ifndef M_SQRT1_2
#define M_SQRT1_2  0.707106781186547524401
#endif // M_SQRT1_2

#define DAMAGE_FRONT    0.334f
#define DAMAGE_SIDE     0.5f
#define DAMAGE_REAR     1.0f
#define DAMAGE_FULL     1.0f

//------------------------------------------------------------------------------
void projectile::touch(object *other, float impulse)
{
    g_World->add_sound(sound_index[BULLET_EXPLODE].name);
    g_World->add_effect(get_position(), effect_type::explosion);

    g_World->remove(this);

    if (!other)
        return;

    if (other->_type == object_type::tank) {
        tank* owner_tank = static_cast<tank*>(_owner);
        tank* other_tank = static_cast<tank*>(other);

        vec2    vOrg, vDir;
        float   flDot, damage;

        if (other_tank->_damage >= 1.0f) {
            return; // dont add damage or score
        }

        vOrg = -get_linear_velocity().normalize();
        vDir = vec2(1,0);
        vDir = rot(vDir,other->get_rotation());
        flDot = (vOrg.x*vDir.x + vOrg.y*vDir.y);

        damage = owner_tank->_client->damage_mod / other_tank->_client->armor_mod;

        if (!g_Game->bExtendedArmor && !g_Game->m_bMultiplayer) {
            other_tank->_damage += damage * DAMAGE_FULL;
        } else if (flDot > M_SQRT1_2) {
            other_tank->_damage += damage * DAMAGE_FRONT;
        } else if (flDot > -M_SQRT1_2) {
            other_tank->_damage += damage * DAMAGE_SIDE;
        } else {
            other_tank->_damage += damage * DAMAGE_REAR;
        }

        if (other_tank->_damage >= 1.0f) {
            g_Game->AddScore( owner_tank->_player_index, 1 );
            other_tank->_dead_time = g_Game->m_flTime;
            if (g_Game->bAutoRestart && !g_Game->m_bMultiplayer)
                g_Game->flRestartTime = g_Game->m_flTime + RESTART_TIME;

            int r = rand()%3;
            switch (r) {
                case 0:
                    g_Game->m_WriteMessage(va("%s couldn't take %s's HEAT.", other_tank->player_name(), owner_tank->player_name()));
                    break;
                case 1:
                    g_Game->m_WriteMessage(va("%s was on the wrong end of %s's cannon.", other_tank->player_name(), owner_tank->player_name()));
                    break;
                case 2:
                    g_Game->m_WriteMessage(va("%s ate all 125mm of %s's boom stick.", other_tank->player_name(), owner_tank->player_name()));
                    break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void projectile::draw() const
{
    float   flLerp = (g_Game->m_flTime - (g_Game->m_nFramenum-1) * FRAMEMSEC) / FRAMEMSEC;
    vec2    p1, p2;

    p1 = get_position( flLerp - 0.4f );
    p2 = get_position( flLerp );

    g_Render->DrawLine( p2, p1, vec4(1,0.5,0,1), vec4(1,0.5,0,0) );
}

} // namespace game
