#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {

    if (seg.header().syn)
    {
        isn = seg.header().seqno;
        syn = true;
        //_reassembler.push_substring("S", 0 ,fin);
    }
    string data = string(seg.payload().str());
    if (seg.header().fin)
    {
        fin = true;
        //data = data + "F";
    }
    _reassembler.push_substring(data, unwrap(seg.header().seqno, isn, _reassembler.stream_out().bytes_written()) ,fin);
    
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{ 
    uint64_t n = _reassembler.stream_out().bytes_written();
    if (syn) 
    {
        if (fin && _reassembler.unassembled_bytes())
            n = n+1;

        return wrap(n , isn);
    } 
    else return nullopt; 
    }

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
