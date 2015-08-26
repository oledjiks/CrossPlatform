#ifndef _ADDRESS_CPP_
#define _ADDRESS_CPP_

#include "Socket.h"

namespace Socket
{
    void Address::_address(Ip ip, Port port)
    {
        this->sin_family = AF_INET;
        this->set_ip(ip);
        this->set_port(port);
    }

    Address::Address()
    {
        _address("0.0.0.0", 0);
    }

    Address::Address(Port port)
    {
        _address("0.0.0.0", port);
    }

    Address::Address(Ip ip, Port port)
    {
        _address(ip, port);
    }

    Address::Address(struct sockaddr_in address)
    {
        _address(inet_ntoa(address.sin_addr), address.sin_port);
    }

    Address::Address(const Address &address)
    {
        this->sin_family = address.sin_family;
        this->sin_addr = address.sin_addr;
        this->sin_port = address.sin_port;
    }

    Ip Address::get_ip(void) const
    {
        return inet_ntoa(this->sin_addr);
    }

    Ip Address::set_ip(Ip ip)
    {
#ifdef WINDOWS
        unsigned long address = inet_addr(ip.c_str());

        if (address == INADDR_NONE)
        {
            std::stringstream error;
            error << "[ip] with [ip=" << ip << "] Invalid ip address provided";
            throw SocketException(error.str());
        }
        else
        {
            this->sin_addr.S_un.S_addr = address;
        }
#else
        if (inet_aton(ip.c_str(), &this->sin_addr) == 0)
        {
            std::stringstream error;
            error << "[ip] with [ip=" << ip << "] Invalid ip address provided";
            throw SocketException(error.str());
        }
#endif
        return this->get_ip();
    }

    Port Address::get_port(void) const
    {
        return ntohs(this->sin_port);
    }

    Port Address::set_port(Port port)
    {
        this->sin_port = htons(port);
        return this->get_port();
    }

    std::ostream& operator<< (std::ostream &out, Address &address)
    {
        out << address.get_ip() << ":" << address.get_port();
        return out;
    }
}

#endif
