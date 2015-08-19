#ifndef _UDP_CPP_
#define _UDP_CPP_

#include "Socket.h"

namespace Socket
{
    UDP::UDP(void) : CommonSocket(SOCK_DGRAM)
    {
    }

    UDP::UDP(const UDP &udp) : CommonSocket()
    {
        this->_socket_id = udp._socket_id;
        this->_opened = udp._opened;
        this->_binded = udp._binded;
    }

    template <class T>
    int UDP::send(Ip ip, Port port, const T *data, size_t len)
    {
        if (!this->_opened) this->open();

        len *= sizeof(T);
        if (len > (SOCKET_MAX_BUFFER_LEN * sizeof(T)))
        {
            std::stringstream error;
            error << "[send] with [ip=" << ip << "] [port=" << port << "] [data=" << data << "] [len=" << len << "] Data length higher then max buffer len";
            throw SocketException(error.str());
        }

        Address address(ip, port);
        int ret;

        if ((ret = sendto(this->_socket_id, (const char*)data, len, 0, (struct sockaddr*)&address, sizeof(struct sockaddr))) == -1)
        {
            std::stringstream error;
            error << "[send] with [ip=" << ip << "] [port=" << port << "] [data=" << data << "] [len=" << len << "] Cannot send";
            throw SocketException(error.str());
        }

        return ret;
    }

    template <class T>
    int UDP::send(Address address, const T *data, size_t len)
    {
        return this->send<T>(address.ip(), address.port(), data, len);
    }

    template <class T>
    int UDP::send(Ip ip, Port port, T data)
    {
        return this->send<T>(ip, port, &data, 1);
    }

    template <class T>
    int UDP::send(Address address, T data)
    {
        return this->send<T>(address.ip(), address.port(), &data, 1);
    }

    template <>
    int UDP::send<std::string>(Ip ip, Port port, std::string data)
    {
        return this->send<char>(ip, port, data.c_str(), data.length() + 1);
    }

    template <>
    int UDP::send<std::string>(Address address, std::string data)
    {
        return this->send<char>(address.ip(), address.port(), data.c_str(), data.length() + 1);
    }

    template <class T>
    inline int UDP::receive(Address *address, T *data, size_t len, unsigned int *received_elements)
    {
        if (!this->_opened) this->open();
        if (!this->_binded) throw SocketException("[receive] Make the socket listening before receiving");

        len *= sizeof(T);
        if (len > (SOCKET_MAX_BUFFER_LEN * sizeof(T)))
        {
            std::stringstream error;
            error << "[send] with [buffer=" << data << "] [len=" << len << "] Data length higher then max buffer length";
            throw SocketException(error.str());
        }

        int received_bytes;
        socklen_t size = sizeof(struct sockaddr);

        if ((received_bytes = recvfrom(this->_socket_id, (char*)data, len, 0, (struct sockaddr*)address, (socklen_t*)&size)) == -1)
        {
            throw SocketException("[receive] Cannot receive");
        }

        *received_elements = (unsigned int)(received_bytes / sizeof(T));

        return received_bytes;
    }

    template <class T>
    Datagram<T*> UDP::receive(T *buffer, size_t len)
    {
        Datagram<T*> ret;

        ret.received_bytes = this->receive<T>(&ret.address, buffer, len, &ret.received_elements);
        ret.data = buffer;

        return ret;
    }

    template <class T, size_t N>
    Datagram<T[N]> UDP::receive(size_t len)
    {
        Datagram<T[N]> ret;
        ret.received_bytes = this->receive<T>(&ret.address, ret.data, len, &ret.received_elements);
        return ret;
    }

    template <class T>
    Datagram<T> UDP::receive(void)
    {
        Datagram<T> ret;
        ret.received_bytes = this->receive<T>(&ret.address, &ret.data, 1, &ret.received_elements);
        return ret;
    }

    template <>
    Datagram<std::string> UDP::receive<std::string>(void)
    {
        Datagram<std::string> ret;
        char buffer[SOCKET_MAX_BUFFER_LEN];

        ret.received_bytes = this->receive<char>(&ret.address, buffer, SOCKET_MAX_BUFFER_LEN, &ret.received_elements);
        ret.data = buffer;

        return ret;
    }

    template <class T>
    Datagram<T*> UDP::receive_timeout(unsigned int sec, T *buffer, size_t len)
    {
        Datagram<T*> ret;
        fd_set rset;
        int ready;
        struct timeval timeout = {(time_t)sec, 0};

        FD_ZERO(&rset);
        FD_SET(this->_socket_id, &rset);

        ready = ::select(this->_socket_id+1, &rset, NULL, NULL, &timeout);
        if (ready == SOCKET_ERROR)
        {
            throw SocketException("[receive_timeout] SOCKET_ERROR");
        }
        else if (ready > 0)
        {
            if (FD_ISSET(_socket_id, &rset))
            {
                ret.received_bytes = this->receive<T>(&ret.address, buffer, len, &ret.received_elements);
                ret.data = buffer;
            }
        }

        return ret;
    }

    template <class T, size_t N>
    Datagram<T[N]> UDP::receive_timeout(unsigned int sec, size_t len)
    {
        Datagram<T[N]> ret;
        fd_set rset;
        int ready;
        struct timeval timeout = {(time_t)sec, 0};

        FD_ZERO(&rset);
        FD_SET(this->_socket_id, &rset);

        ready = ::select(this->_socket_id+1, &rset, NULL, NULL, &timeout);
        if (ready == SOCKET_ERROR)
        {
            throw SocketException("[receive_timeout] select() return SOCKET_ERROR");
        }
        else if (ready > 0)
        {
            if (FD_ISSET(_socket_id, &rset))
            {
                ret.received_bytes = this->receive<T>(&ret.address, &ret.data, len, &ret.received_elements);
            }
        }

        return ret;
    }

    template <class T>
    Datagram<T> UDP::receive_timeout(unsigned int sec)
    {
        Datagram<T> ret;
        fd_set rset;
        int ready;
        struct timeval timeout = {(time_t)sec, 0};

        FD_ZERO(&rset);
        FD_SET(this->_socket_id, &rset);

        ready = ::select(this->_socket_id+1, &rset, NULL, NULL, &timeout);
        if (ready == SOCKET_ERROR)
        {
            throw SocketException("[receive_timeout] select() return SOCKET_ERROR");
        }
        else if (ready > 0)
        {
            if (FD_ISSET(_socket_id, &rset))
            {
                ret.received_bytes = this->receive<T>(&ret.address, &ret.data, 1, &ret.received_elements);
            }
        }

        return ret;
    }

    template <>
    Datagram<std::string> UDP::receive_timeout<std::string>(unsigned int sec)
    {
        Datagram<std::string> ret;
        char buffer[SOCKET_MAX_BUFFER_LEN];
        fd_set rset;
        int ready;
        struct timeval timeout = {(time_t)sec, 0};

        FD_ZERO(&rset);
        FD_SET(this->_socket_id, &rset);

        ready = ::select(this->_socket_id+1, &rset, NULL, NULL, &timeout);
        if (ready == SOCKET_ERROR)
        {
            throw SocketException("[receive_timeout] select() return SOCKET_ERROR");
        }
        else if (ready > 0)
        {
            if (FD_ISSET(_socket_id, &rset))
            {
                ret.received_bytes = this->receive<char>(&ret.address, buffer, SOCKET_MAX_BUFFER_LEN, &ret.received_elements);
                ret.data = buffer;
            }
        }

        return ret;
    }
}

#endif
