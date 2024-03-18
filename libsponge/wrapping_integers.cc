#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.
#define P2_32 static_cast<uint64_t>(1ULL << 32ULL)
template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t temp = (n + isn.raw_value()) % P2_32;  

    return WrappingInt32{temp};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    bool wrapped = (n.raw_value() < isn.raw_value());
    uint64_t temp = (n.raw_value() + P2_32 * wrapped - isn.raw_value());
    uint64_t chp_re = checkpoint % P2_32;
    uint64_t chp_q = checkpoint - chp_re;
    uint64_t differ = temp > chp_re ? temp - chp_re : chp_re - temp;

    if (differ < (P2_32 / 2))
        temp = temp + chp_q;
    else if (temp > chp_re)
        temp = temp + chp_q - P2_32 * (chp_q != 0);
    else
        temp = temp + chp_q + P2_32;

    return temp;
}
