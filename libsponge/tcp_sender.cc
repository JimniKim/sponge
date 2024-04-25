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
    , _outstanding_segments()
    , timeout(_initial_retransmission_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return flight_bytes; }

void TCPSender::fill_window() {
    
    size_t num = _window_size ? _window_size - bytes_in_flight() : 1;
    if (_window_size == 0 && bytes_in_flight() != 0)
        num = 0;
    while (num > 0 && _fin == false) {
        // this means we've already sent the segment with FIN flag
        //if (_stream.eof() && _next_seqno >= _stream.bytes_written() + 2)
        //    return;

        //uint16_t n_bytes_to_send = _window_size - bytes_in_flight();

        // if space left to fill in window is more than the max payload
        // or we sent out more bytes than the window size, n_bytes_to_send overflows

        // if the receiver reports a window size of 0 but we have stuff to send
        //if (_window_size == 0 && bytes_in_flight() == 0)
        //    n_bytes_to_send = 1;

        TCPHeader hdr;
        hdr.seqno = wrap(_next_seqno, _isn);
        if (!start) {
            hdr.syn = true;
            start = true;
            num = num - 1;
        }

        

        // fill as many bytes as we can from the stream
        TCPSegment seg;
        seg.payload() =  _stream.read(min(num, TCPConfig::MAX_PAYLOAD_SIZE));
         num = num - seg.payload().size();

        // include the FIN flag if it fits
        if (_stream.eof() && (num > 0))
        {
            hdr.fin = true;
            _fin = true;
        }
        num = num - hdr.fin;

        seg.header() = hdr;

        // if the segment is empty (no flags or data) don't send
        size_t length = seg.length_in_sequence_space();
        if (length)
        {
            _segments_out.push(seg);
            _outstanding_segments.insert({_next_seqno, seg});
        }
        else
            return;
        
        _next_seqno += length;
        flight_bytes = flight_bytes + length;

        // if the timer isn't running, start it with the original rtto
        if (time_elapsed >= timeout )  // done
        {
            time_elapsed =0;
            timeout =_initial_retransmission_timeout;
        }
           
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
  // update window size even if we get a wacko ackno?
    _window_size = window_size;
    uint64_t new_abs_ackno = unwrap(ackno, _isn, _next_seqno);
    
    if (new_abs_ackno <= _abs_ackno || new_abs_ackno > _next_seqno)
        return;
    
    _abs_ackno = new_abs_ackno;
    for (auto it = _outstanding_segments.begin(); it != _outstanding_segments.end();)
        {
            if (it->first + it->second.length_in_sequence_space() <= _abs_ackno)
            {
                flight_bytes = flight_bytes - it->second.length_in_sequence_space();
                it = _outstanding_segments.erase(it);
            }
            else
                break;
        }
    
        // only restart timer if there are new complete segments confirmed to be received
    
    
    
    time_elapsed =0;
    timeout =_initial_retransmission_timeout;
    _n_consec_retransmissions = 0;

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { //done
    time_elapsed += ms_since_last_tick;
    
    if ((time_elapsed >= timeout) && (!_outstanding_segments.empty())) {
        _segments_out.push(_outstanding_segments.begin()->second);
        if (_window_size > 0) {
            _n_consec_retransmissions++;
            timeout *= 2;
        }
        time_elapsed =0;
        
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _n_consec_retransmissions; }

void TCPSender::send_empty_segment() { //done
    TCPSegment new_seg;
    new_seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(new_seg);
}