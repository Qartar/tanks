// cm_parser.h
//

#pragma once

#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace parser {

//------------------------------------------------------------------------------
class text
{
public:
    text() = default;
    explicit text(char const* string);
    text(char const* begin, char const* end);
    text(text&& other);

    text& operator=(text&& other);

    std::vector<char const*> const& tokens() const { return _tokens; }

protected:
    std::vector<char> _buffer;
    std::vector<char const*> _tokens;
};

} // namespace parser
