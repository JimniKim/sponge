#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t new_index = index;
    string temp;
    string _data;

    if (eof) {
        last_byte = new_index;
        _eof = true;
    }

    if (index < next + _output.remaining_capacity()) {
        _data = data.substr(0, min(data.size(), next + _output.remaining_capacity() - index));
        if (eof && _data != data)
            _eof = false;
    } else
        return;

    if (index < next && index + _data.size() > next)  // partially overlapping
    {
        _data = _data.substr(next - index);
        new_index = next;
        if (eof)
            last_byte = new_index;
    } else if (index < next && index + _data.size() <= next)  // totally overlapping
        return;

    temp = _data;

    for (auto a = unreassem.begin(); a != unreassem.end();) {
        size_t curr_s = a->first;
        size_t curr_f = a->first + a->second.size();
        size_t new_f = new_index + temp.size();

        if (curr_s <= new_index && curr_f >= new_f)
            return;

        if (curr_s <= new_index && new_index <= curr_f && new_f > curr_f) {
            temp = a->second.substr(0, new_index - curr_s) + temp;
            unassem_bytes = unassem_bytes - (curr_f - curr_s);
            new_index = curr_s;
            unreassem.erase(a++);

        } else if (curr_s <= new_f && new_f <= curr_f && new_index < curr_s) {
            temp = temp.substr(0, curr_s - new_index) + a->second;
            if (curr_s == last_byte)
                last_byte = new_index;
            unassem_bytes = unassem_bytes - (curr_f - curr_s);
            unreassem.erase(a++);
        } else if (curr_s >= new_index && curr_f <= new_f) {
            temp = temp;
            unassem_bytes = unassem_bytes - (curr_f - curr_s);
            unreassem.erase(a++);

        } else
            ++a;
    }

    if (new_index == next)
        next = next + _output.write(temp);
    else {
        unreassem.insert({new_index, temp});
        unassem_bytes = unassem_bytes + temp.size();
    }

    if (next >= last_byte && _eof)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const { return unassem_bytes; }

bool StreamReassembler::empty() const { return unreassem.empty(); }
