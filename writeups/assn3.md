Assignment 3 Writeup
=============

My name: [Kim Jimin]

My POVIS ID: [kjm1672]

My student ID (numeric): [20210084]

This assignment took me about [24] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
[In the fill_window() function, a new segment is generated every time in a while
statement, and pushed to the '_segments_out' only when 
'length_in_sequence_space()' is not 0. Send by pushing. At this time, 
outstanding segments are managed by inserting {seqno, segment} into the new
structure 'outstanding_seg' (map). If the window is completely filled or the
stream is empty, the function is terminated. In the ack_received function,
'outstanding_seg' is inspected and if the range is smaller than ackno, the
corresponding segment is deleted. And initialize the RTO time, time_passed, etc.
In the tick function, it was written as written in pdf.]

Implementation Challenges:
[The send_extra test continued to fail. Even if it is eof, if the window size is
insufficient, the fin flag should not be set to 1, but the window size continued
to be stored as a large value. It turned out that when initializing, it was 
stored as a capacity value, and the desired value was not stored in the window
size. After that, when I initialized to 1, it passed without any problems.]

Remaining Bugs:
[I passed all the tests this time, but I think the part that checks the wrong
ackno needs to be corrected a little. If there is a problem in another test 
later, I think I should check this part, too.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
