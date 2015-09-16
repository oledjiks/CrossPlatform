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
    }

    CommonSocket::~CommonSocket(void)
    {
#ifdef WINDOWS
        this->_num_sockets--;
        if (this->_num_sockets == 0)
            WSACleanup();
#endif
    }

    inline SocketId CommonSocket::get_socket_id(void)
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
        {
#ifdef WINDOWS
            closesocket(this->_socket_id);
#else
            shutdown(this->_socket_id, SHUT_RDWR);
            ::close(this->_socket_id);
#endif
        }

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

    int CommonSocket::set_option(int level, int optname, const void* optval, socklen_t optlen)
    {
        int ret = 0;

        if ((ret = ::setsockopt(_socket_id, level, optname, (const char*)optval, optlen)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_option] error";
            throw SocketException(error.str());
        }

        return ret;
    }

    int CommonSocket::get_option(int level, int optname, void* optval, socklen_t* optlen)
    {
        int ret = 0;
        if ((ret = getsockopt(_socket_id, level, optname, (char*)optval, optlen)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_option] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_broadcast(bool isbroadcast)
    {
        int ret = 0;
        if ((ret = ::setsockopt(_socket_id, SOL_SOCKET, SO_BROADCAST, (const char*)&isbroadcast, sizeof(bool))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_broadcast] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_nonblock(bool isnonblock)
    {
        int ret = 0;
#ifdef WINDOWS
        unsigned long arg;
        arg = (isnonblock) ? 1 : 0;
        if ((ret = ioctlsocket(_socket_id, FIONBIO, (unsigned long*)&arg)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_nonblock] error";
            throw SocketException(error.str());
        }
#else
        int flags = fcntl(_socket_id, F_GETFL, 0);
        long arg = (isnonblock) ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
        if ((ret = fcntl(_socket_id, F_SETFL, arg)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_nonblock] error";
            throw SocketException(error.str());
        }
#endif
        return ret;
    }

    int CommonSocket::set_ttl(int ttl)
    {
        int ret = 0;
        if ((ret = setsockopt(_socket_id, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(int))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_ttl] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::get_ttl(int& ttl)
    {
        int ret = 0;
        socklen_t len;
        if ((ret = getsockopt(_socket_id, IPPROTO_IP, IP_TTL, (char*)&ttl, &len)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_ttl] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_multicast_ttl(int ttl)
    {
        int ret = 0;
        if ((ret = setsockopt(_socket_id, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(int))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_multicast_ttl] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_tos(int tos)
    {
        int ret = 0;
        if ((ret = setsockopt(_socket_id, IPPROTO_IP, IP_TOS, (char*)&tos, sizeof(int))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_tos] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::get_tos(int& tos)
    {
        int ret = 0;
        socklen_t len;
        if ((ret = getsockopt(_socket_id, IPPROTO_IP, IP_TOS, (char*)&tos, &len)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_tos] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_timeout(int sendtimeout, int recvtimeout)
    {
        int ret = 0;
        if (sendtimeout >= 0)
        {
            if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_SNDTIMEO, (char*)&sendtimeout, sizeof(int))) == SOCKET_ERROR)
            {
                std::stringstream error;
                error << "[set_timeout] error";
                throw SocketException(error.str());
            }
        }
        if (recvtimeout >= 0)
        {
            if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_SNDTIMEO, (char*)&recvtimeout, sizeof(int))) == SOCKET_ERROR)
            {
                std::stringstream error;
                error << "[set_timeout] error";
                throw SocketException(error.str());
            }
        }
        return ret;
    }

    int CommonSocket::get_timeout(int& sendtimeout, int& recvtimeout)
    {
        int ret = 0;
        socklen_t len;
        if ((ret = getsockopt(_socket_id, SOL_SOCKET, SO_SNDTIMEO, (char*)&sendtimeout, &len)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_timeout] error";
            throw SocketException(error.str());
        }
        if ((ret = getsockopt(_socket_id, SOL_SOCKET, SO_SNDTIMEO, (char*)&recvtimeout, &len)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_timeout] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_buffsize(int sendbuffsize, int recvbuffsize)
    {
        int ret = 0;
        if (sendbuffsize > 0)
        {
            if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_SNDBUF, (char*)&sendbuffsize, sizeof(int))) == SOCKET_ERROR)
            {
                std::stringstream error;
                error << "[set_buffsize] error";
                throw SocketException(error.str());
            }
        }
        if (recvbuffsize > 0)
        {
            if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_RCVBUF, (char*)&recvbuffsize, sizeof(int))) == SOCKET_ERROR)
            {
                std::stringstream error;
                error << "[set_buffsize] error";
                throw SocketException(error.str());
            }
        }

        return ret;
    }

    int CommonSocket::get_buffsize(int& sendbuffsize, int& recvbuffsize)
    {
        int ret = 0;
        socklen_t len;
        if ((ret = getsockopt(_socket_id, SOL_SOCKET, SO_SNDBUF, (char*)&sendbuffsize, &len)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_buffsize] error";
            throw SocketException(error.str());
        }
        if ((ret = getsockopt(_socket_id, SOL_SOCKET, SO_RCVBUF, (char*)&recvbuffsize, &len)) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[get_buffsize] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int CommonSocket::set_dontfragment(bool isdf)
    {
        int ret = 0;
        if ((ret = setsockopt(_socket_id, IPPROTO_IP, IP_DONTFRAGMENT, (const char*)&isdf, sizeof(isdf))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_dontfragment] error";
            throw SocketException(error.str());
        }
        return ret;
    }
}

#endif
