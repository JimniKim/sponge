#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    EthernetFrame send_frame;
    auto a = mapping.find(next_hop_ip);
    if (a != mapping.end()) ///known
    {
        send_frame.header().type = EthernetHeader :: TYPE_IPv4;
        send_frame.header().src = _ethernet_address;
        send_frame.header().dst = a->second->ether;
        send_frame.payload() = dgram.serialize();
        _frames_out.push(send_frame);
        return;
    }
    else
    {
        auto b = already_sent_ARP.find(next_hop_ip);
        if (b != mapping.end() && b->second.queue_time <= 5)
            return;
        
        send_frame.header().type = EthernetHeader :: TYPE_ARP;
        send_frame.header().src = _ethernet_address;
        send_frame.header().dst = ETHERNET_BROADCAST;
        ARPMessage arp;
        arp.opcode = ARPMessage::OPCODE_REQUEST;
        arp.sender_ethernet_address = _ethernet_address;
        arp.sender_ip_address = _ip_address.ipv4_numeric();
        arp.target_ip_address = next_hop_ip;
        send_frame.payload() = arp.serialize();
        _frames_out.push(send_frame);
    }


    
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    
    InternetDatagram result;
    if (!(frame.header().dst == _ethernet_address ||frame.header().dst == ETHERNET_BROADCAST))
        return nullopt;

    if (frame.header().type == EthernetHeader :: TYPE_ARP)
    {
        ARPMessage arp;
        if (arp.parse(Buffer(frame.payload())) == ParseResult::NoError)
        {
            Ethernet_addr temp {arp.sender_ethernet_address, 0};
            auto a = mapping.find (arp.sender_ip_address);
            if (a != mapping.end())
            {
                
            }
            mapping.push({arp.sender_ip_address, temp}); // for 30secs
            
            if (arp.target_ip_address == _ip_address.ipv4_numeric())
                arp.target_ethernet_address = _ethernet_address;
            
            
            EthernetFrame send_frame;
            send_frame.header() = frame.header();
            send_frame.payload() = arp.serialize();
            if (result.parse(frame.payload()) == ParseResult::NoError)
                return result;
        }
            
    }
    else if (frame.header().type == EthernetHeader :: TYPE_IPv4)
    {
        InternetDatagram result;
        if (result.parse(Buffer(frame.payload())) == ParseResult::NoError)
            return result;
    }
    return nullopt;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) 
{ 
    for (auto i)
    {
        if (ms_since_last_tick < )
        expire
    }
}
