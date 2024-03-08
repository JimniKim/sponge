Assignment 1 Writeup
=============

My name: [Kim Jimin]

My POVIS ID: [kjm1672]

My student ID (numeric): [20210084]

This assignment took me about [5] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
[I included the map class (#include <map>) as a temporary buffer to handle
unassembled substrings. The index and data received from the push_substring
function are stored as key and value, respectively. To handle overlapping
substrings, it is largely divided into three cases. 1. When data overlaps 
with the substring of the previous index. 2. When data overlaps with the 
substring of the subsequent index. 3. When data completely contains a 
subsstring. After combining overlapping strings, the temporary buffer is 
examined and the write function is called if the next byte of the stream 
exists.]

Implementation Challenges:
[As mentioned above, the case of overlapped substring was largely divided into
three categories, and the problem was when the two cases occurred together.
Because this case was not considered at first, there was a situation where 
the strings were less assembled. So after declaring a new string 'temp', I went 
around the for loop and continued to add the combined string to this temp. 
Later, after the for loop ended, I proceeded to insert this string at once.]

Remaining Bugs:
[I passed all the tests, but the handling of the overlapped string seems 
to be a little poor. I've considered the least, but if the string comes 
in more complicatedly in the future, I'm afraid I'll get an error.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
