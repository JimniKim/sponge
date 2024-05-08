Assignment 5 Writeup
=============

My name: [Kim Jimin]

My POVIS ID: [kjm1672]

My student ID (numeric): [20210084]

This assignment took me about [10] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
[First, the 'mapping' structure was declared to store the matching and passing 
time between the ip address and the ethernet address. 
In addition, the 'already_sent_ARP' structure was queued with the ip address 
and the ARP sent from that address, and the time was recorded. After that, in 
the 'send_datagram' function, when the destination address is known and when 
it is not known is processed separately. In the case of the 'recv_frame' 
function, it was necessary to distinguish between the case of ARP and the case 
of IPv4. First, parse the payload as arp, and update it if 'arp.
sender_eternet_address' is already in mapping, or add a new one if not. Check 
'already_sent_ARP' to send all Ethernet frames on the wait list at the address. 
After that, if arp is a request, create a new frame for reply and push it.
In the tick function, each of the structures 'mapping' and 'already_sent_ARP' 
is checked with a for statement, and the pass time is increased by 
ms_since_last_tick. If it is 'mapping', it is checked whether the time exceeds 
3000, or if it is 'already_sent_ARP', it is checked whether it exceeds 5000. 
If it exceeds, delete it.]

Implementation Challenges:
[In the 'recv_frame' function, it was easy for the frame to be IPv4, but in the 
case of ARP, it was difficult because there were many things to consider. In 
particular, it was difficult to record the matching in the 'mapping' structure 
and process it effectively for only 30 seconds. This was solved by checking it
later in the tick function.]

Remaining Bugs:
[All of the 'make check_lab5' tests passed, but one test continued to timeout 
when the 'make check' test was conducted. After neatly organizing the repeated 
codes into functions, all tests could be passed.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
