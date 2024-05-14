#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    Router_mem temp {route_prefix, prefix_length, next_hop.has_value() ? next_hop: nullopt, interface_num};
    router_list.push_back(temp);
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    std::list <Router_mem> matched_list;
    for (auto i = router_list.begin(); i != router_list.end();i++)
    {
        uint32_t prefix = dgram.header().dst >> (32 - i->prefix_length);
        if (prefix == i->route_prefix) 
            matched_list.push_back(*i);
        
    }
    if (matched_list.empty() || dgram.header().ttl ==0 || --(dgram.header().ttl))
        return;
    
    if (!matched_list.empty())
    {
        Router_mem longest = matched_list.front();
        for (auto i = matched_list.begin(); i != matched_list.end(); i++)
            if (longest.prefix_length < i->prefix_length)
                longest = *i;

        if (longest.next_hop != nullopt)
            interface(longest.interface_num).send_datagram(dgram,longest.next_hop);
        else
            interface(longest.interface_num).send_datagram(dgram,Address::from_ipv4_numeric(dgram.header().dst));
    }
    
     
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
