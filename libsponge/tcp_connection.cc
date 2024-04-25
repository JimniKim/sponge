#include "tcp_connection.hh"

#include <iostream>



template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _last_segm_recv_timer; }

bool TCPConnection::syn_sent() { return _sender.next_seqno_absolute() > 0; }

bool TCPConnection::active() const { return _active; }

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    try_closing_connection();  // send_segments() called in try_closing_connection()
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_segments();
}

void TCPConnection::segment_received(const TCPSegment &seg) {
   _last_segm_recv_timer =0;
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
        send_segments();
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
            send_segments();
    }

    if (seg.header().syn && !(_sender.next_seqno_absolute() > 0))
        connect();

    //if (_receiver.ackno().has_value() && (seg.length_in_sequence_space()==0 
    //&& (seg.header().seqno == _receiver.ackno().value() -1))) // keep-alives
    //{
    //    _sender.send_empty_segment();
    //}

    bool prereq1 = _receiver.unassembled_bytes() ==0 && inbound_stream().input_ended(); // prereq1
    if (prereq1 && !_sender.stream_in().eof())
        _linger_after_streams_finish = false;
    
    
    return;
    
}

size_t TCPConnection::write(const string &data) {
    size_t num_written = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return num_written;
}

void TCPConnection::reset() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}

void TCPConnection::try_switching_close_mode() {
    if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended())
        _linger_after_streams_finish = false;
}

//\param[in] rst is false by default, true a reset flag needs to be sent out
void TCPConnection::send_segments(bool rst) {
    if (!active())
        return;

    while (!_sender.segments_out().empty()) {
        TCPSegment seg = _sender.segments_out().front();
        _sender.segments_out().pop();

        auto possible_ackno = _receiver.ackno();
        if (possible_ackno.has_value()) {
            seg.header().ack = true;
            seg.header().ackno = possible_ackno.value();
            seg.header().win = _receiver.window_size();
        }
        if (rst)
            seg.header().rst = true;

        _segments_out.push(seg);
        if (debug)
            cout << ">> sending segment: " << seg.header().summary() << endl;
    }
}

void TCPConnection::try_closing_connection() {
    if (!active())
        return;

    if (debug)
        cout << ">> try closing connection: " << endl;
    send_segments();
    bool active_close = _linger_after_streams_finish && _last_segm_recv_timer >= 10 * _cfg.rt_timeout;
    bool passive_close = !_linger_after_streams_finish;

    // prereq 1: inbound stream ended & fully assembled
    bool istream_done = _receiver.stream_out().input_ended() && _receiver.unassembled_bytes() == 0;

    // prereq 2 (and 3): outbound stream ended, fully sent (and fully ack'd by remote peer)
    bool ostream_done = _sender.stream_in().eof() &&
                        _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
                        _sender.bytes_in_flight() == 0;

    if (debug) {
        cout << "   >> linger? " << _linger_after_streams_finish << endl
             << "   >> active close: " << active_close << endl
             << "   >> passive close: " << passive_close << endl
             << "   >> prereq 1: " << istream_done << endl
             << "   >> prereq2,3 " << ostream_done << endl;
    }
    if ((active_close || passive_close) && istream_done && ostream_done) {
        if (debug)
            cout << ">> closing..." << endl;
        _active = false;
    }
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!active())
        return;

    _last_segm_recv_timer = _last_segm_recv_timer + ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    if (_sender.consecutive_retransmissions()> TCPConfig::MAX_RETX_ATTEMPTS)
    {
        send_segments(true);
        _sender.stream_in().set_error();
        inbound_stream().set_error(); // set error state
        _active = false; // kill connection
        //_linger_after_streams_finish = false;
        return;
    }
    else
        send_segments();
    

    if (!active())
        return;

    
    send_segments();

    //bool active_close = _linger_after_streams_finish && _last_segm_recv_timer >= 10 * _cfg.rt_timeout;
    //bool passive_close = !_linger_after_streams_finish;

    // prereq 1: inbound stream ended & fully assembled
    bool prereq1 =inbound_stream().input_ended() && _receiver.unassembled_bytes() == 0;

    // prereq 2 (and 3): outbound stream ended, fully sent (and fully ack'd by remote peer)
    bool prereq2 = _sender.stream_in().eof() && _sender.next_seqno_absolute() == (_sender.stream_in().bytes_written() + 2);
    bool prereq3 = _sender.bytes_in_flight() == 0;

    if (prereq1 && prereq2 && prereq3)
    {
        if (_last_segm_recv_timer >= 10 * _cfg.rt_timeout && _linger_after_streams_finish)
            _active = false;
        else if (!_linger_after_streams_finish)
            _active = false;
    }
    

}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            if (_sender.segments_out().empty())
                _sender.send_empty_segment();
            send_segments(true);
            reset();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}