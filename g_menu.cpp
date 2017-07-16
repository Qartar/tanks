// g_menu.cpp
//

#include "precompiled.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////
namespace menu {

//------------------------------------------------------------------------------
void window::init ()
{
    // base

    _submenus.emplace_back(new menu::window);  // local
    _submenus.emplace_back(new menu::window);  // network
    _submenus.emplace_back(new menu::window);  // options

    add_button<conditional_button>("Resume", vec2(64,32), vec2(96,32), &g_Game->_game_active, [](){
        g_Game->resume();
    });

    add_button<submenu_button>("Network Game", vec2(192,32), vec2(96,32), this, _submenus[1].get());
    add_button<submenu_button>("Local Game", vec2(320,32), vec2(96,32), this, _submenus[0].get());
    add_button<submenu_button>("Game Options", vec2(448,32), vec2(96,32), this, _submenus[2].get());

    add_button<button>("Quit", vec2(576,32), vec2(96,32), [](){
        g_Game->stop_server();
        g_Game->stop_client();
        g_Application->quit(0);
    });

    // local

    _submenus[0]->add_button<button>("New Round", vec2(48,80), vec2(64,32), [](){
        g_Game->stop_server();
        g_Game->stop_client();
        g_Game->new_game();
    });

    _submenus[0]->add_button<button>("Reset", vec2(48,128), vec2(64,32), [](){
        g_Game->reset();
    });

    // network

    _submenus[1]->_submenus.emplace_back(new menu::window);
    _submenus[1]->_submenus.emplace_back(new menu::window);
    _submenus[1]->add_button<submenu_button>("Host", vec2(48,80), vec2(64,24), _submenus[1].get(), _submenus[1]->_submenus[0].get());
    _submenus[1]->add_button<submenu_button>("Join", vec2(128,80), vec2(64,24), _submenus[1].get(), _submenus[1]->_submenus[1].get());

    // network->host

    _submenus[1]->_submenus[0]->add_button<host_button>(vec2(144,128), vec2(256,24), [](){
        g_Game->_dedicated = false;
        g_Game->start_server();
    });

    // network->join

    _submenus[1]->_submenus[1]->add_button<button>("Refresh", vec2(240,80), vec2(64,24), [](){
        g_Game->info_ask( );
    });

    for (int ii = 0; ii < 8; ++ii) {
        _submenus[1]->_submenus[1]->add_button<server_button>(vec2(144,128+ii*32), vec2(256,24), g_Game->cls.servers[ii].name, &g_Game->cls.servers[ii].ping, [ii](){
            g_Game->connect_to_server(ii);
        });
    }

    // options

    _submenus[2]->add_button<client_button>("Player", vec2(64,104), vec2(96,80), &g_Game->cls.color);
}

//------------------------------------------------------------------------------
void window::shutdown()
{
    _buttons.clear();
    _submenus.clear();
}

//------------------------------------------------------------------------------
bool window::activate(submenu_button* button, menu::window* submenu)
{
    if (submenu == _active_menu)
    {
        _active_button = nullptr;
        _active_menu = nullptr;
        return false;   // deactivates caller
    }

    if (_active_button) {
        _active_button->deactivate();
    }
    _active_button = button;
    _active_menu = submenu;

    return true;
}

//------------------------------------------------------------------------------
void window::draw(render::system* renderer, vec2 cursor_pos) const
{
    // draw buttons
    for (auto const& button : _buttons) {
        button->draw(renderer, cursor_pos);
    }

    // draw submenu
    if (_active_menu) {
        _active_menu->draw(renderer, cursor_pos);
    }
}

//------------------------------------------------------------------------------
void window::click(vec2 cursor_pos, bool down)
{
    for (auto& button : _buttons) {
        button->click(cursor_pos, down);
    }
    if (_active_menu) {
        _active_menu->click(cursor_pos, down);
    }
}

} // namespace menu
