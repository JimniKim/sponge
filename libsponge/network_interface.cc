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
        send_frame.header().dst = a->second.ether;
        send_frame.payload() = dgram.serialize();
        _frames_out.push(send_frame);
        return;
    }
    else
    {
        auto b = already_sent_ARP.find(next_hop_ip);
        if (b != already_sent_ARP.end() && b->second.queue_time <= 5000)
            return;
        else if (b != already_sent_ARP.end())
            b->second.waiting_apply.push(dgram);
        else if (b == already_sent_ARP.end())
        {
            Sent_arp temp;
            temp.queue_time=0;
            temp.waiting_apply.push(dgram);
            already_sent_ARP.insert({next_hop_ip, temp});
        }
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
    
    
    if (!(frame.header().dst == _ethernet_address ||frame.header().dst == ETHERNET_BROADCAST))
        return nullopt;

    if (frame.header().type == EthernetHeader :: TYPE_ARP)
    {
        ARPMessage arp;
        if (arp.parse(Buffer(frame.payload())) == ParseResult::NoError)
        {
            auto a = mapping.find (arp.sender_ip_address);
            if (a != mapping.end())
            {
                a ->second.passing_time =0;
                a ->second.ether = arp.sender_ethernet_address;
            }
            else
            {
                Ethernet_addr temp {arp.sender_ethernet_address, 0};
                mapping.insert({arp.sender_ip_address, temp}); // for 30secs
            }
            
            auto b = already_sent_ARP.find(arp.sender_ip_address);
            if (b!= already_sent_ARP.end())
            while (!(b -> second.waiting_apply.empty()))
            {
                EthernetFrame temp_ether;

                temp_ether.header().type = EthernetHeader :: TYPE_IPv4;
                temp_ether.header().src = _ethernet_address;
                temp_ether.header().dst = arp.sender_ethernet_address;
                temp_ether.payload() = b->second.waiting_apply.front().serialize();
                _frames_out.push(temp_ether);
               
                b -> second.waiting_apply.pop();
            }

            if (arp.opcode == arp.OPCODE_REQUEST && arp.target_ip_address == _ip_address.ipv4_numeric())
            {
                EthernetFrame send_frame;
                ARPMessage new_arp;
                new_arp.opcode = new_arp.OPCODE_REPLY;
                new_arp.sender_ip_address = _ip_address.ipv4_numeric();
                new_arp.sender_ethernet_address = _ethernet_address;
                new_arp.target_ip_address = arp.sender_ip_address;
                new_arp.target_ethernet_address = arp.sender_ethernet_address;


                send_frame.header().type = EthernetHeader :: TYPE_ARP;
                send_frame.header().src = _ethernet_address;
                send_frame.header().dst = arp.sender_ethernet_address;
    
                
                send_frame.payload() = new_arp.serialize();
                _frames_out.push(send_frame);

            }
            
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
    for (auto i = mapping.begin(); i != mapping.end();) 
    {
        i->second.passing_time += ms_since_last_tick;
        if (i->second.passing_time >= 30000)
            i = mapping.erase(i);
        else 
            i++;

    }


    for (auto i =already_sent_ARP.begin(); i != already_sent_ARP.end();) 
    {
        i->second.queue_time += ms_since_last_tick;
        if (i->second.queue_time >= 5000)
            i = already_sent_ARP.erase(i);
        else 
            i++;

    }

}
