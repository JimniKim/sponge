Assignment 6 Writeup
=============

My name: [Kim Jimin]

My POVIS ID: [kjm1672]

My student ID (numeric): [20210084]

This assignment took me about [5] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
[I made a Router_mem struct to store route_prefix, prefix_length, 
next_hop_ip, interface_num, next_hop_empty. 
After that, 'std::list<Router_mem>router_list{};' 
was declared to create a router table. 
In the 'add_route' function, values received as parameters were 
stored in the 'Router_mem' structure and added to the 'router_list'. 
In the function 'route_one_datagram', routers matching the 'dst' address 
of the 'dgram' in the 'router_list' were added to the'matched_list'. 
After that, in the 'matched_list', the router with the longest prefix_length 
is found, and a datagram is sent to the appropriate interface.]

Implementation Challenges:
[If the 'prefix_length' is 0, the 'prefix' calculation of 'dst' of 
'dgram' was not properly performed. 
To solve this problem, if the 'prefix_length' is 0, 
the 'prefix' value is set to 0. Then it worked properly.]

Remaining Bugs:
[I passed both the 'make check_lab6' and 'make check' tests. 
However, while modifying the code, I printed out 'dgram.header().ttl', 
and '@' came out. Looking at the 'ipv4_header.hh' file, ttl was 
initialized to 'DEFAULT_TTL', which is declared as 
'static constexprouint8_t DEFAULT_TTL=128'. 
I would like to know why it is output as @ later.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
