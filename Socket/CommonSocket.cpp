#ifndef _COMMON_SOCKET_CPP_
#define _COMMON_SOCKET_CPP_

#include "Socket.h"

namespace Socket
{
#ifdef WINDOWS
    unsigned int CommonSocket::_num_sockets = 0;
#endif

    void CommonSocket::_socket(void)
    {
#ifdef WINDOWS
        _num_sockets++;
        if (_num_sockets == 1)
        {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
                throw SocketException("[constructor] Cannot initialize socket environment");
        }
#endif
    }

    CommonSocket::CommonSocket(void)
    {
        CommonSocket::_socket();
    }

    CommonSocket::CommonSocket(int socket_type)
    {
        CommonSocket::_socket();

        this->_socket_type = socket_type;
        this->_opened = false;
        this->_binded = false;
        this->open();
    }

    CommonSocket::~CommonSocket(void)
    {
#ifdef WINDOWS
        this->_num_sockets--;
        if (this->_num_sockets == 0)
            WSACleanup();
#endif
    }

    SocketId CommonSocket::get_socket_id(void)
    {
        return this->_socket_id;
    }

    void CommonSocket::open(void)
    {
        if (!this->_opened)
        {
            if ((this->_socket_id = socket(AF_INET, this->_socket_type, 0)) == -1)
                throw SocketException("[open] Cannot create socket");
            this->_opened = true;
            this->_binded = false;
        }
    }

    void CommonSocket::close(void)
    {
        if (this->_opened)
#ifdef WINDOWS
            closesocket(this->_socket_id);
#else
            shutdown(this->_socket_id, SHUT_RDWR);
            ::close(this->_socket_id);
#endif

        this->_opened = false;
        this->_binded = false;
    }

    void CommonSocket::bind_on_port(Port port)
    {
        if (this->_binded)
            throw SocketException("[bind_on_port] Socket already binded to a port, close the socket before to re-bind");

        if (!this->_opened)
            this->open();

        Address address(port);

        if (bind(this->_socket_id, (struct sockaddr*)&address, sizeof(struct sockaddr)) == -1)
        {
            std::stringstream error;
            error << "[bind_on_port] with [port=" << port << "] Cannot bind socket";
            throw SocketException(error.str());
        }

        this->_binded = true;
    }

    int CommonSocket::set_option(int level, int optname, const char *optval, socklen_t optlen)
    {
        int ret;

        if ((ret = ::setsockopt(_socket_id, level, optname, (const char *)optval, optlen)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_option] error";
            throw SocketException(error.str());
        }

        return ret;
    }
}

#endif
