#ifndef _COMMON_SOCKET_CPP_
#define _COMMON_SOCKET_CPP_

#include "Socket.h"
#if __cplusplus == 201103
#include <unordered_map>
#endif

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
#endif

        this->_opened = false;
        this->_binded = false;
    }

    void CommonSocket::listen_on_port(Port port)
    {
        if (this->_binded)
            throw SocketException("[listen_on_port] Socket already binded to a port, close the socket before to re-bind");

        if (!this->_opened)
            this->open();

        Address address(port);

        if (bind(this->_socket_id, (struct sockaddr*)&address, sizeof(struct sockaddr)) == -1)
        {
            std::stringstream error;
            error << "[listen_on_port] with [port=" << port << "] Cannot bind socket";
            throw SocketException(error.str());
        }

        this->_binded = true;
    }

    int CommonSocket::set_option(int level, int optname, const char *optval, socklen_t optlen)
    {
        int ret;

#if __cplusplus == 201103
        std::unordered_map<int, std::string> optname_str = {
            // SOL_SOCKET options
            {SO_DEBUG , "SO_DEBUG"},
            {SO_ACCEPTCONN , "SO_ACCEPTCONN"},
            {SO_REUSEADDR , "SO_REUSEADDR"},
            {SO_KEEPALIVE , "SO_KEEPALIVE"},
            {SO_DONTROUTE , "SO_DONTROUTE"},
            {SO_BROADCAST , "SO_BROADCAST"},
            {SO_USELOOPBACK , "SO_USELOOPBACK"},
            {SO_LINGER , "SO_LINGER"},
            {SO_OOBINLINE , "SO_OOBINLINE"},
            {SO_SNDBUF , "SO_SNDBUF"},
            {SO_RCVBUF , "SO_RCVBUF"},
            {SO_SNDLOWAT , "SO_SNDLOWAT"},
            {SO_RCVLOWAT , "SO_RCVLOWAT"},
            {SO_SNDTIMEO , "SO_SNDTIMEO"},
            {SO_RCVTIMEO , "SO_RCVTIMEO"},
            {SO_ERROR , "SO_ERROR"},
            {SO_TYPE , "SO_TYPE"},
#ifdef WINDOWS
            {SO_BSP_STATE , "SO_BSP_STATE"},
            {SO_GROUP_ID , "SO_GROUP_ID"},
            {SO_GROUP_PRIORITY , "SO_GROUP_PRIORITY"},
            {SO_MAX_MSG_SIZE , "SO_MAX_MSG_SIZE"},
            {SO_CONDITIONAL_ACCEPT , "SO_CONDITIONAL_ACCEPT"},
            {SO_PAUSE_ACCEPT , "SO_PAUSE_ACCEPT"},
            {SO_COMPARTMENT_ID , "SO_COMPARTMENT_ID"},
            {SO_RANDOMIZE_PORT , "SO_RANDOMIZE_PORT"},
#endif

            // PROTOCOL_IP options
            {IP_OPTIONS , "IP_OPTIONS"},
            {IP_HDRINCL , "IP_HDRINCL"},
            {IP_TOS , "IP_TOS"},
            {IP_TTL , "IP_TTL"},
            {IP_MULTICAST_IF , "IP_MULTICAST_IF"},
            {IP_MULTICAST_TTL , "IP_MULTICAST_TTL"},
            {IP_MULTICAST_LOOP , "IP_MULTICAST_LOOP"},
            {IP_ADD_MEMBERSHIP , "IP_ADD_MEMBERSHIP"},
            {IP_DROP_MEMBERSHIP , "IP_DROP_MEMBERSHIP"},
            {IP_DONTFRAGMENT , "IP_DONTFRAGMENT"},
            {IP_ADD_SOURCE_MEMBERSHIP , "IP_ADD_SOURCE_MEMBERSHIP"},
            {IP_DROP_SOURCE_MEMBERSHIP , "IP_DROP_SOURCE_MEMBERSHIP"},
            {IP_BLOCK_SOURCE , "IP_BLOCK_SOURCE"},
            {IP_UNBLOCK_SOURCE , "IP_UNBLOCK_SOURCE"},
            {IP_PKTINFO , "IP_PKTINFO"},
            {IP_UNICAST_IF , "IP_UNICAST_IF"},
#ifdef WINDOWS
            {IP_HOPLIMIT , "IP_HOPLIMIT"},
            {IP_RECEIVE_BROADCAST , "IP_RECEIVE_BROADCAST"},
            {IP_RECVIF , "IP_RECVIF"},
            {IP_RECVDSTADDR , "IP_RECVDSTADDR"},
            {IP_IFLIST , "IP_IFLIST"},
            {IP_ADD_IFLIST , "IP_ADD_IFLIST"},
            {IP_DEL_IFLIST , "IP_DEL_IFLIST"},
            {IP_RTHDR , "IP_RTHDR"},
            {IP_RECVRTHDR , "IP_RECVRTHDR"},
#endif
        };
#endif  // for c++11

        if ((ret = ::setsockopt(_socket_id, level, optname, (const char *)optval, optlen)) == SOCKET_ERROR)
        {
            std::stringstream error;
#if __cplusplus == 201103
            error << "[set_option] error (" << optname_str[optname] << ")";
#else
            error << "[set_option] error (" << optname << ")";
#endif
            throw SocketException(error.str());
        }

        return ret;
    }
}

#endif
