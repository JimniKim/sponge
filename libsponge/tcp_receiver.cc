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
    }
    
    if (seg.header().fin)
    {
        fin = true;
    }
    string data = seg.payload().str().to_string();
    uint64_t index = unwrap(seg.header().seqno, isn, _reassembler.stream_out().bytes_written());
    _reassembler.push_substring(data, index ,fin);
    
}

optional<WrappingInt32> TCPReceiver::ackno() const { if (syn) {return wrap(_reassembler.stream_out().bytes_written()+1, isn);} else return nullopt; }

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
