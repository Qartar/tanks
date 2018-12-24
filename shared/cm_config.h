// cm_config.h
//

#pragma once

#include "cm_shared.h"

#include <memory>
#include <string>
#include <map>

////////////////////////////////////////////////////////////////////////////////
namespace config {

class system;
class variable;

enum flags
{
    archive     = 1 << 0,
    server      = 1 << 1,
    reset       = 1 << 2,
};

enum class value_type { string, integer, boolean, scalar };
static char const* type_strings[] = { "string", "integer", "boolean", "scalar" };

//------------------------------------------------------------------------------
class variable_base
{
public:
    variable_base(char const* name, char const* value, value_type type, int flags, char const* description)
        : _name(name)
        , _value(value)
        , _type(type)
        , _flags(flags)
        , _description(description)
    {}
    variable_base(char const* name, char const* value, char const* type_string, char const* flags_string, char const* description);

    char const* name() const { return _name.c_str(); }
    char const* description() const { return _description.c_str(); }
    std::string const& value() const { return _value; }
    int flags() const { return _flags; }
    value_type type() const { return _type; }
    void set(char const* value) { _value = value; }

protected:
    friend system;
    friend variable;

    std::string _name;
    std::string _description;
    std::string _value;
    int _flags;
    value_type _type;
};

//------------------------------------------------------------------------------
class variable
{
public:
    char const* name() const { return _base->name(); }
    char const* description() const { return _base->description(); }
    std::string const& value() const { _base->value(); }
    int flags() const { return _base->flags(); }
    value_type type() const { return _base->type(); }
    void set(char const* value) { _base->set(value); }

protected:
    friend system;

    std::shared_ptr<variable_base> _base;

    variable* _next;
    static variable* _head;

protected:
    variable(char const* name, char const* value, value_type type, int flags, char const* description);

    std::string& get_string() const;
    int get_integer() const;
    bool get_boolean() const;
    float get_scalar() const;

    void set_string(std::string const& s);
    void set_integer(int i);
    void set_boolean(bool b);
    void set_scalar(float f);

    std::string to_string(int i) const;
    std::string to_string(bool b) const;
    std::string to_string(float f) const;
};

//------------------------------------------------------------------------------
class string : public variable
{
public:
    string(char const* name, char const* value, int flags, char const* description)
        : variable(name, value, value_type::string, flags, description)
    {}

    operator char const*() const;
    operator std::string const&() const;
    config::string& operator=(char const* s);
    config::string& operator=(std::string const& s);
};

//------------------------------------------------------------------------------
class integer : public variable
{
public:
    integer(char const* name, int value, int flags, char const* description)
        : variable(name, to_string(value).c_str(), value_type::integer, flags, description)
    {}

    operator int() const;
    config::integer& operator=(int i);
};

//------------------------------------------------------------------------------
class boolean : public variable
{
public:
    boolean(char const* name, bool value, int flags, char const* description)
        : variable(name, to_string(value).c_str(), value_type::boolean, flags, description)
    {}

    operator bool() const;
    config::boolean& operator=(bool b);
};

//------------------------------------------------------------------------------
class scalar : public variable
{
public:
    scalar(char const* name, float value, int flags, char const* description)
        : variable(name, to_string(value).c_str(), value_type::scalar, flags, description)
    {}

    operator float() const;
    config::scalar& operator=(float f);
};

//------------------------------------------------------------------------------
class system
{
public:
    system();
    ~system();

    void init();
    void shutdown();

    variable_base* find(char const* name);
    char const* get(char const* name);
    bool set(char const* name, char const* value);

    void list() const;

    static system* singleton() { return _singleton; }

protected:
    friend variable;

    struct insensitive_compare {
        bool operator()(std::string const& lhs, std::string const& rhs) const {
            return _stricmp(lhs.c_str(), rhs.c_str()) < 0;
        }
    };

    std::map<std::string, std::shared_ptr<variable_base>, insensitive_compare> _variables;

    static system* _singleton;

protected:
    variable_base* find(char const* name, value_type type);
    char const* print(variable_base const* base, int tab_size = 4) const;

    std::shared_ptr<variable_base> create(char const* name, char const* value, value_type type, int flags, char const* description);
};

} // namespace config
