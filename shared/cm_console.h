// cm_console.h
//

#pragma once

#include <cstdlib>
#include <array>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace file {
class stream;
};

//------------------------------------------------------------------------------
class console_buffer
{
public:
    console_buffer();

    void append(char const* begin, char const* end);
    void resize(std::size_t num_columns);
    void write(file::stream& steam) const;

    std::size_t num_rows() const { return _rows_begin - _rows_end; }
    char const* get_row(std::size_t index) const {
        std::size_t row = _rows[(_rows_begin - index - 1) & _rows_mask];
        return &_buffer[row & buffer_mask];
    }

protected:
    static constexpr std::size_t buffer_size = 1 << 16;
    static constexpr std::size_t buffer_mask = buffer_size - 1;

    std::array<char, buffer_size> _buffer;

    std::size_t _offset;

    std::vector<std::size_t> _rows;
    std::size_t _rows_mask; //!<
    std::size_t _rows_begin; //!< index of next row
    std::size_t _rows_end; //!< index of earliest row

    std::size_t _row_offset; //!< number of chars in the current row
    std::size_t _row_columns; //!< number of columns in the current row

    std::size_t _max_columns; //!< maximum number of columns in each row

protected:
    void append_row(char const* begin, char const* end);
    void append_endline();

    std::size_t num_columns(char const* begin, char const* end) const;
    char const* advance_columns(std::size_t columns, char const* begin, char const* end) const;
};

//------------------------------------------------------------------------------
class console_input
{
public:
    console_input();

    void clear();

    bool char_event(int key);
    bool key_event(int key, bool down);

    char const* begin() const { return _buffer.data(); }
    char const* end() const { return _buffer.data() + _length; }
    char const* cursor() const { return _buffer.data() + _cursor; }

protected:
    static constexpr std::size_t buffer_size = 256;
    std::array<char, buffer_size> _buffer;

    std::size_t _length;
    std::size_t _cursor;
    bool _control;
};
