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
    
    Router_mem temp {route_prefix, prefix_length, next_hop.has_value() ? next_hop.value().ipv4_numeric() : 0, interface_num, next_hop.has_value()};
    //cout <<"route_prefix: " <<route_prefix <<"\n";
    //cout <<"prefix_length: "<< int(prefix_length) << "\n";
    //if (next_hop.has_value())
    //    temp = Router_mem{route_prefix, prefix_length, next_hop.ipv4_numeric(), interface_num,false};
    //else 
    //    temp= Router_mem {route_prefix, prefix_length, 0, interface_num,true};
    router_list.push_back(temp);
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    std::list <Router_mem> matched_list{};
    int max_length = -1;
    uint32_t next_ip =0;
    bool next_hop_empty = false;
    for (auto i = router_list.begin(); i != router_list.end();i++)
    {
        uint8_t length = i->prefix_length;
        uint32_t prefix = length ? (dgram.header().dst >> (32 - length))<<(32 - length) : 0;
        
        uint32_t route_prefix = (i->route_prefix >> (32 - length))<<(32 - length);

        cout << "prefix: " << Address::from_ipv4_numeric(prefix).ip() << " " 
        << "i->route_prefix: " << Address::from_ipv4_numeric(i->route_prefix).ip() <<" " 
        "i->prefix_length: " << int(i->prefix_length) << "\n";
        cout << "route_prefix: " <<Address::from_ipv4_numeric (route_prefix).ip()<<"\n";
        if (prefix == i->route_prefix && max_length <= i-> prefix_length) 
        {
            cout << "match!" <<"prefix: " << Address::from_ipv4_numeric(prefix).ip() << " " 
        << "i->route_prefix: " << Address::from_ipv4_numeric(i->route_prefix).ip() <<" " 
        "i->prefix_length: " << int(i->prefix_length) << "\n";
            cout << "route_prefix: " <<Address::from_ipv4_numeric (route_prefix).ip()<<"\n"; 
            max_length = i-> prefix_length;
            next_ip = i -> next_hop_ip;
            Router_mem temp = *i;
            matched_list.push_back(temp);
            next_hop_empty = i -> next_hop_empty;
        }
        
    }
    cout << "\n\n";
    cout << "max_length: " <<max_length<< " next_ip: " <<Address::from_ipv4_numeric(next_ip).ip()
    <<" next_hop_empty: " << next_hop_empty <<"\n";
    cout << "matched_list.empty(): " <<matched_list.empty() << " dgram.header().ttl: " << int(dgram.header().ttl)<<"\n";
    if (max_length == -1 || matched_list.empty() || dgram.header().ttl <= 1)
        return;
    --(dgram.header().ttl);
    cout << " dgram.header().ttl: " << dgram.header().ttl<<"\n";
    if ( max_length != -1||!matched_list.empty())
    {
        Router_mem longest = matched_list.front();
        for (auto i = matched_list.begin(); i != matched_list.end(); i++)
            if (longest.prefix_length < i->prefix_length)
                longest = *i;
        cout << "longest.interface_num: " <<longest.interface_num<<'\n';
        //if (longest.next_hop_empty)
        if (next_hop_empty)
            interface(longest.interface_num).send_datagram(dgram,Address::from_ipv4_numeric(next_ip));
            //interface(longest.interface_num).send_datagram(dgram,Address::from_ipv4_numeric(longest.next_hop_ip));
           
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
