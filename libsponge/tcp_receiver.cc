#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!syn && !seg.header().syn) 
        return;

    else if (seg.header().syn)
    {
        isn = seg.header().seqno;
        syn = true;
    }
        

    //if (seg.header().fin && _reassembler.stream_out().input_ended()) 
    //    fin = true;
    
    uint64_t abs_index = unwrap(seg.header().seqno- + seg.header().syn, isn, _reassembler.stream_out().bytes_written());
    _reassembler.push_substring(seg.payload().copy(), abs_index - 1, seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (syn) 
        return wrap(_reassembler.stream_out().bytes_written() + 1 + _reassembler.stream_out().input_ended(), isn);
   
    return nullopt;
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }