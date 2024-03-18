#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn) {
        isn = seg.header().seqno;
        syn = true;
    }
    
    if (seg.header().fin) {
        fin = true;
        abs_seq_fin = unwrap(
            seg.header().seqno + seg.length_in_sequence_space() - 1, isn, _reassembler.stream_out().bytes_written());
    }
    _reassembler.push_substring(
        string(seg.payload().str()),
        unwrap(seg.header().seqno - 1 * (!seg.header().syn), isn, _reassembler.stream_out().bytes_written()),
        fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    uint64_t n = _reassembler.stream_out().bytes_written() + 1;
    if (syn) {
        if (fin && abs_seq_fin == n)
            n = n + 1;

        return wrap(n, isn);
    } else
        return nullopt;
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
