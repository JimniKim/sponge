#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) :stream_capacity(capacity),total_written (0),
total_read(0), curr_size (capacity), curr_left_size(0), the_end (false),buffer("") {}


size_t ByteStream::write(const string &data) 
{
    if (the_end)
        return 0;

    int curr;
    if (data.size() <= curr_left_size)
    {
        buffer << data;
        curr_left_size = curr_left_size - data.size();
        curr_size = stream_capacity - curr_left_size;
        return data.size();
    }
    else
    {
        buffer << data.substr (0, curr_left_size);
        curr = curr_left_size;
        curr_left_size = 0;
        curr_size = stream_capacity - curr_left_size;
        return curr;
    }
    
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const 
{
    string copy_buffer = buffer.str();
    string peek = copy_buffer.substr(0,len);
    return peek;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) 
{
    string pop_buffer = buffer.str().substr(len);
    buffer.clear();
    buffer.str("");
    buffer << pop_buffer;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) 
{
    string read_bytes = peek_output (len);
    pop_output(len);
    return read_bytes;
}

void ByteStream::end_input() 
{
    the_end = true;
}

bool ByteStream::input_ended() const { return the_end; }

size_t ByteStream::buffer_size() const { return curr_size; }

bool ByteStream::buffer_empty() const { return curr_left_size == stream_capacity; }

bool ByteStream::eof() const 
{ 
    if (buffer_empty()&&input_ended())
        return true;
    else 
        return false;
}

size_t ByteStream::bytes_written() const { return total_written; }

size_t ByteStream::bytes_read() const { return total_read; }

size_t ByteStream::remaining_capacity() const { return curr_left_size; }
