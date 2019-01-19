// cm_parser.h
//

#pragma once

#include "cm_string.h"
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace parser {

//------------------------------------------------------------------------------
class text
{
public:
    text() = default;
    explicit text(string::view string);
    text(char const* begin, char const* end)
        : text(string::view{begin, end})
    {}
    text(text&& other);

    text& operator=(text&& other);

    std::vector<string::view> const& tokens() const { return _tokens; }

protected:
    std::vector<char> _buffer;
    std::vector<string::view> _tokens;
};

} // namespace parser
