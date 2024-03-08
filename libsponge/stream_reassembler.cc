#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity)
,unassem_bytes(0),next(0), last_byte(0),unreassem() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {

    
    
    if (eof)
        last_byte = index;

    
    if (index < next && index+data.size() > next) // partially overlapping
    {
        next = next + _output.write(data.substr(next-index));
    }
    else if (index < next && index+data.size() <= next) // totally overlapping
        return;
    else if (index >= next)
    {
        size_t new_index = index;
        string temp = data;
        
        if (unassem_bytes + data.size () > _capacity)
            temp = data.substr(0,_capacity - unassem_bytes);
        
        for (auto a = unreassem.begin(); a != unreassem.end();)
        {
            if (a->first >= new_index && a->first +a->second.size() >= new_index+ temp.size())
                return;

            if (a->first <= new_index &&  new_index <= a->first + a->second.size()-1 &&
            new_index + temp.size() > a->first + a->second.size() )  
                {
                    temp = a->second.substr(0,new_index-a->first) + temp;
                    unassem_bytes = unassem_bytes - a->second.size();
                    unreassem.erase(a++);
                    new_index = a->first;
                }
            else if (a->first <=  new_index + temp.size()-1 &&  new_index + temp.size()-1 <= a->first + a->second.size()-1 &&
            new_index < a->first)
                {
                    temp = temp.substr(0,a->first - new_index) + a->second;
                    new_index = new_index;
                    if (a-> first == last_byte)
                        last_byte = new_index;
                    unassem_bytes = unassem_bytes - a->second.size();
                    unreassem.erase(a++);
                }
            else if (a->first <= new_index && a->first +a->second.size() <= new_index+ temp.size())
            {
                temp = temp;
                unassem_bytes = unassem_bytes - a->second.size();
                unreassem.erase(a++);
                new_index = new_index;
            }
            else
                ++a;
            
        }
        unreassem.insert({new_index,temp});
        unassem_bytes = unassem_bytes + temp.size();
        
    }

    if (unreassem.find (next) == unreassem.end())
        return;
    else
        {
            next = next + _output.write (unreassem[next]);
            if (next == last_byte)
                _output.end_input();
            unreassem.erase(next);
        }

}

size_t StreamReassembler::unassembled_bytes() const { return unassem_bytes; }

bool StreamReassembler::empty() const { return unreassem.empty(); }
