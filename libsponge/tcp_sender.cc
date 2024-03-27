#include "tcp_sender.hh"

#include "tcp_config.hh"

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
    , _window_size (0)
    , _ackno(wrap(1,_isn).raw_value())
    , consecutive_retran(0) {}

uint64_t TCPSender::bytes_in_flight() const 
{ 
     std::queue<TCPSegment> copy_seg =  outstanding_seg;
    uint64_t count = 0;
    for (int i =0; i < outstanding_seg.size(); i++)
        count = count + outstanding_seg[i].length_in_sequence_space();
    return count;
}

void TCPSender::fill_window() 
{
    string new_string = _stream.read( _window_size);
    uint16_t num =  new_string.size(); 
    while (string.size()!= 0)
    {
        num = min(TCPConfig::MAX_PAYLOAD_SIZE, new_string.size());
        TCPSegment new_seg;
        new_seg.payload(new_string.substr(0, num));
        new_string = new_string.substr(num);
        outstanding_seg.push_back(new_seg);
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{
    _window_size = window_size;
    _ackno = ackno.raw_value();
    outstanding_seg.pop_front();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{
    if (ms_since_last_tick > _initial_retransmission_timeout)


}

unsigned int TCPSender::consecutive_retransmissions() const {return consecutive_retran}

void TCPSender::send_empty_segment() 
{
    TCPSegment new_seg;
    new_seg.header().seqno = _ackno;
    _segments_out.push(new_seg);
}
