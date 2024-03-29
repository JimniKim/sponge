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
    , _window_size (capacity)
    , _ackno(wrap(1,_isn).raw_value())
    , consecutive_retran(0)
    , seq(_isn.raw_value())
    , rto(_initial_retransmission_timeout)
    , time_passed (0)
    , timer (false)
    , start(false) {}

uint64_t TCPSender::bytes_in_flight() const 
{ 
    uint64_t count = 0;
    for (auto i = outstanding_seg.begin(); i != outstanding_seg.end(); i++)
        count = count + i->second.length_in_sequence_space();
    return count;
}

void TCPSender::fill_window() 
{
    
    long unsigned int num = _window_size;

    if (_window_size == 0)
        num = 1;

    while (num > 0 )
    {
        TCPSegment new_seg;

        if (start == false)
        {
            new_seg.header().syn =true;
            start = true;
        }

                
         new_seg.header().seqno = wrap(_next_seqno, _isn);

        num = num-new_seg.header().syn;

       

        unsigned int min_num = min(min(_stream.buffer_size(),TCPConfig::MAX_PAYLOAD_SIZE), num);
        new_seg.payload() = Buffer (_stream.read(min_num)); 
        num = num - new_seg.payload().str().size();

         if (num  > 0   && _stream.eof())
            new_seg.header().fin = true;
        
        num = num - new_seg.header().fin;


        if (new_seg.length_in_sequence_space() != 0)
        {
            _segments_out.push(new_seg);
            outstanding_seg.insert({_next_seqno, new_seg}); 
        }
            

        seq = seq + new_seg.length_in_sequence_space();
        _next_seqno = _next_seqno + new_seg.length_in_sequence_space();
        //num = num - new_seg.payload().str().size();

        if (outstanding_seg.empty())
        {
            rto = _initial_retransmission_timeout;
            time_passed =0;
        }
        
        if (_stream.buffer_empty() || num <=0)
            break;
    }




}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{
    
    if (seq < ackno.raw_value() || ackno.raw_value() <= _ackno)
        return;
    

    _window_size = window_size;
    _ackno = ackno.raw_value();

    rto = _initial_retransmission_timeout;
    consecutive_retran =0;
    time_passed =0;

    if (outstanding_seg.empty())
        timer = true;

    for (auto i = outstanding_seg.begin(); i != outstanding_seg.end();)
    {
        if (i->first +i->second.length_in_sequence_space() <= unwrap (ackno, _isn, _next_seqno))
            i = outstanding_seg.erase(i);
        else
            ++i;
    }
    
        

    if (_window_size > 0)
        fill_window();
    
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{
    time_passed = time_passed + ms_since_last_tick;

    if ((time_passed >= rto) && (!outstanding_seg.empty()))
    {
        _segments_out.push(outstanding_seg.begin()->second);
        if (_window_size > 0)
        {
            consecutive_retran++;
            rto = rto*2;
            timer = true;
        }

        time_passed =0;
    }

}

unsigned int TCPSender::consecutive_retransmissions() const {return consecutive_retran;}

void TCPSender::send_empty_segment() 
{
    TCPSegment new_seg;
    new_seg.header().seqno = wrap(_next_seqno, _isn);
    //if (start == false)
    //        new_seg.header().syn = true;
    //start = true;
    _segments_out.push(new_seg);
}
