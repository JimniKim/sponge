Assignment 4 Writeup
=============

My name: [Kim Jimin]

My POVIS ID: [kjm1672]

My student ID (numeric): [20210084]

This assignment took me about [28] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.41, 0.37]

Program Structure and Design of the TCPConnection:
[When rst was received, unclear shutdown was performed by immediately making all
ByteStreams into error state and storing the _active variable as false.
A function to actually send a segment was implemented and called in the
segment_received, write, tick, end_input_stream, and connect() functions. In
addition, in case of sending an rst segment, a function to send an rst segment 
was separately implemented so that unclear shutdown could be performed in 
~TCConnectio(), tick. Finally, the code for checking clean shutdown 
was inserted into segment_received, tick so that it could be terminated 
without error.]

Implementation Challenges:
[No matter how much tcp_connection.cc was modified, only 76% to 77% of the test
passed, and the rest took time out. It was thought that the cause was the part
that was operating inefficiently in the sender, receiver, and reassembler.
Accordingly, the code was modified efficiently and more neatly in the order of
sender, receiver, and reassembler. In doing this, the number of tests passed
gradually increased, and eventually, all of them could pass. This task felt the
most difficult because it was not just a problem for connection, 
but the operation of all modules had to be considered.]

Remaining Bugs:
[I passed all the tests, but I think I need to check more if there is no
inefficient code because it may not be in the test of the next assignment.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
