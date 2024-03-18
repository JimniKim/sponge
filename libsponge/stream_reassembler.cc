#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), unassem_bytes(0), next(1), last_byte(0), unreassem(), _eof(false) {}

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
        // next = next + _output.write(_data.substr(next-index));
        // if (index == last_byte && _eof)
        //        _output.end_input();
    } else if (index < next && index + _data.size() <= next)  // totally overlapping
        return;

    temp = _data;

    for (auto a = unreassem.begin(); a != unreassem.end();) {
        if (a->first <= new_index && a->first + a->second.size() >= new_index + temp.size())
            return;

        if (a->first <= new_index && new_index <= a->first + a->second.size() &&
            new_index + temp.size() > a->first + a->second.size()) {
            temp = a->second.substr(0, new_index - a->first) + temp;
            unassem_bytes = unassem_bytes - a->second.size();
            new_index = a->first;
            unreassem.erase(a++);

        } else if (a->first <= new_index + temp.size() && new_index + temp.size() <= a->first + a->second.size() &&
                   new_index < a->first) {
            temp = temp.substr(0, a->first - new_index) + a->second;
            new_index = new_index;
            if (a->first == last_byte)
                last_byte = new_index;
            unassem_bytes = unassem_bytes - a->second.size();
            unreassem.erase(a++);
        } else if (a->first >= new_index && a->first + a->second.size() <= new_index + temp.size()) {
            temp = temp;
            unassem_bytes = unassem_bytes - a->second.size();
            new_index = new_index;
            unreassem.erase(a++);

        } else
            ++a;
    }
    unreassem.insert({new_index, temp});
    unassem_bytes = unassem_bytes + temp.size();

    if (unreassem.find(next) == unreassem.end())
        return;
    else {
        size_t end = next;
        next = next + _output.write(unreassem[next]);
        unassem_bytes = unassem_bytes - unreassem[end].size();
        if (end == last_byte && _eof)
            _output.end_input();
        unreassem.erase(end);
    }
}

size_t StreamReassembler::unassembled_bytes() const { return unassem_bytes; }

bool StreamReassembler::empty() const { return unreassem.empty(); }
