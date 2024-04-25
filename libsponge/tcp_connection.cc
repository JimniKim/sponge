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
    last_time =0;
    if (seg.header().rst)
    {
        _sender.stream_in().set_error();
        inbound_stream().set_error(); // set error state
        _active = false; // kill connection
        //_linger_after_streams_finish = false;
        return;
    }

    _receiver.segment_received(seg);

    if (seg.header().ack) 
    {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        really_send_seg();
    }
    if (seg.length_in_sequence_space()) 
    {
        _sender.fill_window();
        if (_sender.segments_out().empty())
        {   
            _sender.send_empty_segment();
            TCPSegment new_seg = _sender.segments_out().front();
            _sender.segments_out().pop();
            if (_receiver.ackno().has_value())
            {
                new_seg.header().ack = true;
                new_seg.header().ackno = _receiver.ackno().value();
            }
            new_seg.header().win= static_cast<uint16_t>(_receiver.window_size());
            _segments_out.push (new_seg);

        }
        else
            really_send_seg();
    }

    //if (seg.header().syn && !(_sender.next_seqno_absolute() > 0))
    //    connect();

    //if (_receiver.ackno().has_value() && (seg.length_in_sequence_space()==0 
    //&& (seg.header().seqno == _receiver.ackno().value() -1))) // keep-alives
    //{
    //    _sender.send_empty_segment();
    //}

    bool prereq1 = _receiver.unassembled_bytes() ==0 && inbound_stream().input_ended(); // prereq1
    bool prereq2 = _sender.stream_in().eof() && _sender.next_seqno_absolute() == (_sender.stream_in().bytes_written() + 2);
    bool prereq3 = _sender.bytes_in_flight() == 0;

    if (prereq1 && !_sender.stream_in().eof())
        _linger_after_streams_finish = false;
    
    if (prereq1 && prereq2 && prereq3)
    {
        if (last_time >= 10 * _cfg.rt_timeout && _linger_after_streams_finish)
            _active = false;
        else if (!_linger_after_streams_finish)
            _active = false;
    }

    return;
    
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) 
{ 
    size_t num_written = _sender.stream_in().write(data);
    _sender.fill_window();
    really_send_seg();
    return num_written;
    
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) 
{ 
    last_time = last_time + ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions()> TCPConfig::MAX_RETX_ATTEMPTS)
    {
        really_send_seg_rst();
        _sender.stream_in().set_error();
        inbound_stream().set_error(); // set error state
        _active = false; // kill connection
        //_linger_after_streams_finish = false;
        return;
    }
    else if (_receiver.ackno().has_value()) { 
        _sender.fill_window();
    }

    really_send_seg();
    

    bool prereq1 = inbound_stream().input_ended() && _receiver.unassembled_bytes() == 0;
    bool prereq2 = _sender.stream_in().eof() && _sender.next_seqno_absolute() == (_sender.stream_in().bytes_written() + 2);
    bool prereq3 = _sender.bytes_in_flight() == 0; 
    
    if (prereq1 && prereq2 && prereq3)
    {
        if (last_time >= 10 * _cfg.rt_timeout && _linger_after_streams_finish)
            _active = false;
        else if (!_linger_after_streams_finish)
            _active = false;
    }

}

void TCPConnection::end_input_stream() 
{
    _sender.stream_in().end_input();
    _sender.fill_window();
    really_send_seg();
    
}

void TCPConnection::connect() 
{
    _sender.fill_window();
    really_send_seg();
    
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            
            really_send_seg_rst();
            _sender.stream_in().set_error();
            inbound_stream().set_error(); // set error state
            _active = false; // kill connection
            
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::really_send_seg_rst()
{
    if (_sender.segments_out().empty())
        _sender.send_empty_segment();
    TCPSegment new_seg = _sender.segments_out().front();
    _sender.segments_out().pop();
    if (_receiver.ackno().has_value())
    {
        new_seg.header().ack = true;
        new_seg.header().ackno = _receiver.ackno().value();
    }
    new_seg.header().win= static_cast<uint16_t>(_receiver.window_size());
    new_seg.header().rst = true;
    _segments_out.push (new_seg);
    
}
void TCPConnection:: really_send_seg()
{

    while (_sender.segments_out().empty() == false)
    {
        TCPSegment new_seg = _sender.segments_out().front();
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value())
        {
            new_seg.header().ack = true;
            new_seg.header().ackno = _receiver.ackno().value();
        }

        new_seg.header().win= static_cast<uint16_t>(_receiver.window_size());
        
        _segments_out.push (new_seg);

    }
    
}