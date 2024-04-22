#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return last_time; }

void TCPConnection::segment_received(const TCPSegment &seg) 
{  
    _receiver.segment_received(seg); 
    //
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) 
{ 
    size_t num_written = _sender.stream_in().write(data);
    return num_written;
    //
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) 
{ 
    // 
}

void TCPConnection::end_input_stream() 
{
    _sender.stream_in().end_input();
    //
}

void TCPConnection::connect() 
{
    _sender.fill_window();
    while (_sender.segments_out().empty() != true)
    {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        auto _ackno = _receiver.ackno();
        if (_ackno.has_value())
        {
            seg.header().ack = true;
            seg.header().ackno = _ackno.value();
        }
        seg.header().win= static_cast<uint16_t>(_receiver.window_size());
    
        _segments_out.push(seg);
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            _sender.send_empty_segment();
            TCPSegment new_seg = _sender.segments_out().front();
            _sender.segments_out().pop();

            _sender.stream_in().end_input();
            inbound_stream().end_input();
            really_send_seg_rst();
            new_seg.header().rst = true;
            _segments_out.push(new_seg);
            _active = false;
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::really_send_seg_rst()
{
    _sender.send_empty_segment();
    TCPSegment new_seg = _sender.segments_out().front();
    _sender.segments_out().pop();
    auto _ackno = _receiver.ackno();
    if (_ackno.has_value())
    {
        new_seg.header().ack = true;
        new_seg.header().ackno = _ackno.value();
    }
    new_seg.header().win= static_cast<uint16_t>(_receiver.window_size());
    new_seg.header().rst = true;
    _segments_out.push (new_seg);
    
}