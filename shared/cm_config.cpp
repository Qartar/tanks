//  cm_config.cpp
//

#include "cm_config.h"
#include "cm_filesystem.h"
#include "cm_parser.h"

#include <Shlobj.h>
#include <PathCch.h>

////////////////////////////////////////////////////////////////////////////////
namespace config {

variable* variable::_head = nullptr;
system* system::_singleton = nullptr;

namespace {

//------------------------------------------------------------------------------
value_type type_from_string(char const* type_string)
{
    for (size_t ii = 0; ii < countof(type_strings); ++ii) {
        if (strcmp(type_string, type_strings[ii]) == 0) {
            return static_cast<value_type>(ii);
        }
    }
    return value_type::string;
}

} // anonymous namespace

//------------------------------------------------------------------------------
variable_base::variable_base(char const *name, char const* value, char const* type_string, char const* flags_string, char const* description)
    : _name(name)
    , _value(value)
    , _type(type_from_string(type_string))
    , _flags(atoi(flags_string))
    , _description(description)
{}

//------------------------------------------------------------------------------
void variable_base::set(char const* value)
{
    char* end = nullptr;

    switch (type()) {
        case value_type::string: {
            set_string(value);
            break;
        }

        case value_type::integer: {
            long d = strtol(value, &end, 0);
            if (end != value) {
                set_integer(d);
            } else {
                log::message("cannot set variable '^fff%s^xxx' to non-integer value '^fff%s^xxx'\n", name(), value);
            }
            break;
        }

        case value_type::boolean: {
            if (!_stricmp(value, "true") || !strcmp(value, "1")) {
                set_boolean(true);
            } else if (!_stricmp(value, "false") || !strcmp(value, "0")) {
                set_boolean(false);
            } else {
                log::message("cannot set variable '^fff%s^xxx' to non-boolean value '^fff%s^xxx'\n", name(), value);
            }
            break;
        }

        case value_type::scalar: {
            float f = strtof(value, &end);
            if (end != value) {
                set_scalar(f);
            } else {
                log::message("cannot set variable '^fff%s^xxx' to non-scalar value '^fff%s^xxx'\n", name(), value);
            }
            break;
        }
    }
}

//------------------------------------------------------------------------------
void variable_base::set_string(std::string const& s)
{
    _value = s;
    _flags |= flags::modified;
}

//------------------------------------------------------------------------------
void variable_base::set_integer(int i)
{
    set_string(to_string(i));
}

//------------------------------------------------------------------------------
void variable_base::set_boolean(bool b)
{
    set_string(to_string(b));
}

//------------------------------------------------------------------------------
void variable_base::set_scalar(float f)
{
    set_string(to_string(f));
}

//------------------------------------------------------------------------------
std::string variable_base::to_string(int i) const
{
    return va("%d", i);
}

//------------------------------------------------------------------------------
std::string variable_base::to_string(bool b) const
{
    return b ? "true" : "false";
}

//------------------------------------------------------------------------------
std::string variable_base::to_string(float f) const
{
    std::string s = va("%f", f);
    if (s.find('.') != 0) {
        // strip trailing zeroes
        while (s.back() == '0') {
            s.pop_back();
        }
        // strip trailing decimal point
        if (s.back() == '.') {
            s.pop_back();
        }
    }
    return s;
}

//------------------------------------------------------------------------------
std::string const& variable_base::get_string() const
{
    return _value;
}

//------------------------------------------------------------------------------
int variable_base::get_integer() const
{
    return atoi(_value.c_str());
}

//------------------------------------------------------------------------------
bool variable_base::get_boolean() const
{
    return _value == "true" ? true
        : _value == "false" ? false
        : atoi(_value.c_str()) != 0; 
}

//------------------------------------------------------------------------------
float variable_base::get_scalar() const
{
    return static_cast<float>(std::atof(_value.c_str()));
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
variable::variable(char const* name, char const* value, value_type type, int flags, char const* description)
{
    if (system::_singleton) {
        _base = system::_singleton->create(name, value, type, flags, description);
        _next = nullptr;
    } else {
        _base = std::make_shared<variable_base>(name, value, type, flags, description);
        _next = _head;
        _head = this;
    }
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
string::operator char const*() const
{
    return get_string().c_str();
}

//------------------------------------------------------------------------------
string::operator std::string const&() const
{
    return get_string();
}

//------------------------------------------------------------------------------
config::string& string::operator=(char const* s)
{
    set_string(s);
    return *this;
}

//------------------------------------------------------------------------------
config::string& string::operator=(std::string const& s)
{
    set_string(s);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
integer::operator int() const
{
    return get_integer();
}

//------------------------------------------------------------------------------
config::integer& integer::operator=(int i)
{
    set_integer(i);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
boolean::operator bool() const
{
    return get_boolean(); 
}

//------------------------------------------------------------------------------
config::boolean& boolean::operator=(bool b)
{
    set_boolean(b);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
scalar::operator float() const
{
    return get_scalar();
}

//------------------------------------------------------------------------------
config::scalar& scalar::operator=(float f)
{
    set_scalar(f);
    return *this;
}

////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
int get_config_path(char *path, std::size_t size, bool create = false)
{
    PWSTR pszPath;
    WCHAR wPath[1024];

    SHGetKnownFolderPath( FOLDERID_LocalAppData, KF_FLAG_DEFAULT, NULL, &pszPath );

    PathCchCombine(wPath, countof(wPath), pszPath, L"Tanks!");

    if (create) {
        CreateDirectoryW(wPath, NULL);
    }

    PathCchAppend(wPath, countof(wPath), L"config.ini" );

    CoTaskMemFree( pszPath );

    return WideCharToMultiByte(
        CP_ACP,
        WC_NO_BEST_FIT_CHARS,
        wPath,
        -1,
        path,
        narrow_cast<int>(size),
        NULL,
        NULL);
}

//------------------------------------------------------------------------------
char const* system::print(variable_base const* base, int /*tab_size*/) const
{
    static char buffer[MAX_STRING];

    sprintf(buffer, "%-20s %-20s %-8s %3d \"%s\"\n",
         base->name(),
         va("\"%s\"", base->value().c_str()),
         config::type_strings[static_cast<int>(base->type())],
         base->flags(),
         base->description() );

    return buffer;
}

//------------------------------------------------------------------------------
system::system()
{
    assert(_singleton == nullptr);
    _singleton = this;
}

//------------------------------------------------------------------------------
system::~system()
{
    assert(_singleton == this);
    _singleton = nullptr;
}

//------------------------------------------------------------------------------
void system::init()
{
    // iterate over all variables initialized before the config system
    {
        for (auto var = variable::_head; var; var = var->_next) {
            auto it = _variables.find(var->_base->_name);
            if (it != _variables.end()) {
                assert(it->second != var->_base);
                assert(it->second->type() == var->type());
                // point all instances of the variable to the same base
                var->_base = it->second;
            } else {
                // add variable base to dictionary
                _variables[var->_base->_name] = var->_base;
            }
        }
        config::variable::_head = nullptr;
    }

    // load saved configuration from ini file
    {
        char filename[LONG_STRING];

        get_config_path(filename, countof(filename));
        file::buffer buffer = file::read(filename);

        char const* ptr = (char const*)buffer.data();
        char const* end = ptr + buffer.size();

        while(ptr && ptr < end) {
            char const* next = strchr(ptr, '\n');
            parser::text text(ptr, next);

            ptr = next ? next + 1 : nullptr;
            if (text.tokens().size() < 2) {
                break;
            }

            auto it = _variables.find(text.tokens()[0]);
            if (it != _variables.end()) {
                // update existing variable
                auto type = type_from_string(text.tokens()[2]);
                if (type != it->second->_type) {
                    log::warning("archived variable '^fff%s^xxx' has wrong type, reverting to default\n", text.tokens()[0]);
                } else {
                    it->second->_value = text.tokens()[1];
                }
            } else {
                // create new variable
                _variables[text.tokens()[0]] = std::make_shared<variable_base>(
                    text.tokens()[0],
                    text.tokens()[1],
                    text.tokens()[2],
                    text.tokens()[3],
                    text.tokens()[4]);
            }
        }
    }
}

//------------------------------------------------------------------------------
void system::shutdown()
{
    char filename[LONG_STRING];
    char const* line;

    get_config_path(filename, countof(filename), true);

    if (filename) {
        file::stream file = file::open(filename, file::mode::write);

        for (auto it : _variables) {
            if (it.second->flags() & config::archive) {
                line = print(it.second.get(), 4);
                file.write((byte const*)line, strlen(line));
            }
        }
    }
}

//------------------------------------------------------------------------------
void system::command_list(parser::text const& /*args*/)
{
    for (auto it : _singleton->_variables) {
        log::message(_singleton->print(it.second.get(), 4));
    }
}

//------------------------------------------------------------------------------
variable_base* system::find(char const* name)
{
    auto it = _variables.find(name);
    if (it != _variables.end()) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

//------------------------------------------------------------------------------
variable_base* system::find(char const* name, value_type type)
{
    auto it = _variables.find(name);
    if (it != _variables.end() && it->second->type() == type) {
        return it->second.get();
    } else {
        return nullptr;
    }
}

//------------------------------------------------------------------------------
std::shared_ptr<variable_base> system::create(char const* name, char const* value, value_type type, int flags, char const* description)
{
    auto it = _variables.find(name);
    if (it != _variables.end()) {
        return it->second;
    } else {
        auto base = std::make_shared<variable_base>(name, value, type, flags, description);
        _variables[name] = base;
        return base;
    }
}

//------------------------------------------------------------------------------
char const* system::get(char const* name)
{
    auto it = _variables.find(name);
    if (it != _variables.end()) {
        return it->second->value().c_str();
    } else {
        return nullptr;
    }
}

//------------------------------------------------------------------------------
bool system::set(char const* name, char const* value)
{
    auto it = _variables.find(name);
    if (it != _variables.end()) {
        it->second->set(value);
        return true;
    } else {
        return false;
    }
}

} // namespace config
