#include "tcp_sender.hh"

#include "tcp_config.hh"

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
    , _window_size(1)
    , _ackno(wrap(0, _isn).raw_value())
    , consecutive_retran(0)
    , seq(_isn.raw_value())
    , rto(_initial_retransmission_timeout)
    , time_passed(0)
    , start(false)
    , _fin(false)
    , flight_bytes(0) {}

uint64_t TCPSender::bytes_in_flight() const { return flight_bytes; }

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

        string payload_read = _stream.read(min(num, TCPConfig::MAX_PAYLOAD_SIZE));
        new_seg.payload() = Buffer(std::move(payload_read));

        num = num - new_seg.payload().size();

        if (_stream.eof() && (num > 0)) {
            new_seg.header().fin = true;
            _fin = true;
        }

        num = num - new_seg.header().fin;

        if (new_seg.length_in_sequence_space() != 0) {
            _segments_out.push(new_seg);
            outstanding_seg.insert({_next_seqno, new_seg});
        }

        seq = seq + new_seg.length_in_sequence_space();
        _next_seqno = _next_seqno + new_seg.length_in_sequence_space();
        flight_bytes = flight_bytes + new_seg.length_in_sequence_space();

        if (outstanding_seg.empty()) {
            rto = _initial_retransmission_timeout;
            time_passed = 0;
        }

        if (_stream.buffer_empty() || num <= 0)
            break;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    if (ackno.raw_value() == _ackno.raw_value()) {
        _window_size = window_size;
        _ackno = ackno;
        return;
    } else if (seq < ackno.raw_value() ||
               (seq != ackno.raw_value() &&
                (_ackno.raw_value() + (outstanding_seg.begin()->second.length_in_sequence_space()) !=
                 ackno.raw_value())))
        return;


    while (!outstanding_seg.empty())
    {
        auto i = outstanding_seg.begin();
        if (i->first + i->second.length_in_sequence_space() <= unwrap(ackno, _isn, _next_seqno)) {
            flight_bytes = flight_bytes - i->second.length_in_sequence_space();
            outstanding_seg.erase(i);
        }
        else
            break;
    }   
    /* 
    for (auto i = outstanding_seg.begin(); i != outstanding_seg.end();) {
        if (i->first + i->second.length_in_sequence_space() <= unwrap(ackno, _isn, _next_seqno)) {
            flight_bytes = flight_bytes - i->second.length_in_sequence_space();
            i = outstanding_seg.erase(i);
        } else
            break;
    }
    */

    _window_size = window_size;
    _ackno = ackno;

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
