#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <algorithm>
#include <iostream>
#include <random>

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
    , rto(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _next_seqno - absolute_ackno; }

void TCPSender::fill_window() {
    while (_next_seqno <= absolute_ackno + _window_size) {
        // this means we've already sent the segment with FIN flag
        if (_stream.eof() && _next_seqno >= _stream.bytes_written() + 2)
            return;

        uint16_t n_bytes_to_send = _window_size - bytes_in_flight();

        // if space left to fill in window is more than the max payload
        // or we sent out more bytes than the window size, n_bytes_to_send overflows

        // if the receiver reports a window size of 0 but we have stuff to send
        if (_window_size == 0 && bytes_in_flight() == 0)
            n_bytes_to_send = 1;

        TCPHeader hdr;
        hdr.seqno = wrap(_next_seqno, _isn);
        if (_next_seqno == 0 && n_bytes_to_send > 0) {
            hdr.syn = true;
            n_bytes_to_send--;
        }

        if (n_bytes_to_send > TCPConfig::MAX_PAYLOAD_SIZE)
            n_bytes_to_send = TCPConfig::MAX_PAYLOAD_SIZE;

        // fill as many bytes as we can from the stream
        TCPSegment seg;
        seg.payload() = _stream.read(n_bytes_to_send);
        n_bytes_to_send = n_bytes_to_send == TCPConfig::MAX_PAYLOAD_SIZE ? 1 : n_bytes_to_send - seg.payload().size();

        // include the FIN flag if it fits
        if (_stream.eof() && (n_bytes_to_send > 0))
            hdr.fin = true;

        seg.header() = hdr;

        // if the segment is empty (no flags or data) don't send
        if (seg.length_in_sequence_space() == 0)
            return;
        _segments_out.push(seg);
        outstanding_seg.insert({_next_seqno, seg});
        _next_seqno += seg.length_in_sequence_space();

        // if the timer isn't running, start it with the original rtto
        if (time_passed >= rto)
        {
            rto = _initial_retransmission_timeout;
            time_passed = 0;
        }
            
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _window_size = window_size;  // update window size even if we get a wacko ackno?

    uint64_t abs_new_ack = unwrap(ackno, _isn, absolute_ackno);
    if (abs_new_ack <= absolute_ackno || abs_new_ack > _next_seqno)
        return;
    absolute_ackno = abs_new_ack;

    for (auto i = outstanding_seg.begin(); i != outstanding_seg.end();) {
        if (i->first + i->second.length_in_sequence_space() <= absolute_ackno) {
            flight_bytes = flight_bytes - i->second.length_in_sequence_space();
            i = outstanding_seg.erase(i);
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