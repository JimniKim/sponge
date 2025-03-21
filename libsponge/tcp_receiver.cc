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
    else if (seg.header().syn) {
        isn = seg.header().seqno;
        syn = true;
    }

    _reassembler.push_substring(
        seg.payload().copy(),
        unwrap(seg.header().seqno - 1 * (!seg.header().syn), isn, _reassembler.stream_out().bytes_written()),
        seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!syn)
        return {};

    uint64_t n = _reassembler.stream_out().bytes_written() + 1;
    if (_reassembler.stream_out().input_ended())
        n = n + 1;
    return wrap(n, isn);
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
