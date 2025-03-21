#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <iostream>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;
//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , outstanding_seg()
    , rto(_initial_retransmission_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - absolute_ackno; }

void TCPSender::fill_window() {
    size_t num = _window_size ? _window_size - bytes_in_flight() : 1;

    if (_window_size == 0 && bytes_in_flight() != 0)
        num = 0;

    while (num > 0 && _fin == false) {
        TCPSegment new_seg;

        if (start == false) {
            new_seg.header().syn = true;
            start = true;
            num = num - 1;
        }

        new_seg.header().seqno = wrap(_next_seqno, _isn);

        new_seg.payload() = _stream.read(min(num, TCPConfig::MAX_PAYLOAD_SIZE));
        num = num - new_seg.payload().size();

        if (_stream.eof() && (num > 0)) {
            new_seg.header().fin = true;
            _fin = true;
        }

        num = num - new_seg.header().fin;

        size_t length = new_seg.length_in_sequence_space();
        if (length) {
            _segments_out.push(new_seg);
            outstanding_seg.insert({_next_seqno, new_seg});
        }

        else
            return;

        _next_seqno = _next_seqno + length;
        flight_bytes = flight_bytes + length;

        if (time_passed >= rto) {
            rto = _initial_retransmission_timeout;
            time_passed = 0;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t new_ack = unwrap(ackno, _isn, _next_seqno);
    _window_size = window_size;

    if (new_ack <= absolute_ackno || (new_ack <= absolute_ackno && new_ack >= outstanding_seg.begin()->first) ||
        new_ack > _next_seqno)
        return;

    absolute_ackno = new_ack;

    for (auto it = outstanding_seg.begin(); it != outstanding_seg.end();) {
        if (it->first + it->second.length_in_sequence_space() <= absolute_ackno) {
            flight_bytes = flight_bytes - it->second.length_in_sequence_space();
            it = outstanding_seg.erase(it);
        } else
            break;
    }

    // only restart timer if there are new complete segments confirmed to be received

    rto = _initial_retransmission_timeout;
    consecutive_retran = 0;
    time_passed = 0;

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    time_passed = time_passed + ms_since_last_tick;

    if ((time_passed >= rto) && (!outstanding_seg.empty())) {
        _segments_out.push(outstanding_seg.begin()->second);
        if (_window_size > 0) {
            consecutive_retran++;
            rto = rto * 2;
        }

        time_passed = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consecutive_retran; }

void TCPSender::send_empty_segment() {
    TCPSegment new_seg;
    new_seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(new_seg);
}