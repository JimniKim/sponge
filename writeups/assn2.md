Assignment 2 Writeup
=============

My name: [Kim jimin]

My POVIS ID: [kjm1672]

My student ID (numeric): [20210084]

This assignment took me about [4] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[1. TCP Receiver:reassembler starts at index 0, but when unwrap, the index 
does not fit because of SYN. Therefore, seqno-1 was used to allow string 
to enter the byte stream from index 0.
2. Wrap/unwrap: 2^32 is defined as "#define P2_32 static_cast<uint64_t>
(1ULL<32ULL)". The wrap is made to add the two numbers and then take the 
remainder divided by 2^32.
Unwrap was calculated by determining whether or not to add 2^32 depending on 
whether the difference between the two numbers (difference between n and isn, 
checkpoint) divided by 2^32 was less than 2^22.]

Implementation Challenges:
[push_substring did not work normally because absolute seqno and stream index 
were different. So when obtaining the index to enter push_substring, seqno -1, 
not seqno, was substituted into unwrap so that the stream index, not absolute 
seqno, could enter.]

Remaining Bugs:
[All the given tests passed.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
