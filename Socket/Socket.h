/* @file           Socket.h
 * @copyright      HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
 * @brief          Machine Vision Component Socket class
 *
 * @author         zhenglinjun
 * @date           2015-08-01
 *
 * @note
 *
 * @warning
 */
#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <iostream>
#include <sstream>
#include <exception>
#include <string>
#include <vector>
#include <fstream>

#if defined __WIN32 || defined __WIN64 || defined WIN32 || defined WIN64
#define WINDOWS
#endif

#ifdef WINDOWS
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#endif

#ifndef WINDOWS
#define SOCKET_ERROR              (-1)
#endif

#ifndef SOCKET_MAX_BUFFER_BYTES
#define SOCKET_MAX_BUFFER_BYTES   4096
#endif

namespace Socket
{
    typedef int SocketId;
    typedef std::string Ip;
    typedef unsigned int Port;
#ifdef WINDOWS
    typedef int socklen_t;
#endif

    class SocketException : public std::exception
    {
    private:
        std::string         _error;

    public:
        SocketException(const std::string&);
        ~SocketException() throw();

        virtual const char* what() const throw();
        friend std::ostream& operator<< (std::ostream &out, SocketException &e);
    };

    struct Address : protected sockaddr_in
    {
    private:
        void _address(Ip, Port);

    public:
        Address();
        Address(Port);
        Address(Ip, Port);
        Address(struct sockaddr_in);
        Address(const Address&);

        Ip get_ip(void) const;
        Ip set_ip(Ip);

        Port get_port(void) const;
        Port set_port(Port);

        friend std::ostream& operator<< (std::ostream&, Address&);
    };

    template <class DataType>
    struct Datagram
    {
    public:
        Address             address;
        DataType            data;
        unsigned int        received_bytes;
        unsigned int        received_elements;

        Datagram();

        template <class T>
        void operator= (const Datagram<T>&);
    };

    class CommonSocket
    {
    private:
#ifdef WINDOWS
        static unsigned int _num_sockets;
#endif
        static void _socket(void);

    protected:
        SocketId            _socket_id;
        int                 _socket_type;
        bool                _opened;
        bool                _binded;

    public:
        CommonSocket(void);
        CommonSocket(int);

        ~CommonSocket(void);

        SocketId get_socket_id(void);

        void open(void);
        void close(void);

        virtual void bind_on_port(Port);
        int set_option(int, int, const char *, socklen_t);
    };

    class UDP : public CommonSocket
    {
    public:
        UDP(void);
        UDP(const UDP&);

        template <class T> int send(Ip, Port, const T*, size_t);
        template <class T> int send(const Address&, const T*, size_t);
        template <class T> int send(Ip, Port, T);
        template <class T> int send(const Address&, T);

    public:
        template <class T> inline int receive(Address&, T*, size_t, unsigned int&);
        template <class T> inline int receive_timeout(unsigned int, Address&, T*, size_t, unsigned int&);

    public:
        template <class T> Datagram<T*> receive(T*, size_t len = SOCKET_MAX_BUFFER_BYTES / sizeof(T));
        template <class T, size_t N> Datagram<T[N]> receive(size_t len = N);
        template <class T> Datagram<T> receive(void);

        template <class T> Datagram<T*> receive_timeout(unsigned int, T*, size_t len = SOCKET_MAX_BUFFER_BYTES / sizeof(T));
        template <class T, size_t N> Datagram<T[N]> receive_timeout(unsigned int, size_t len = N);
        template <class T> Datagram<T> receive_timeout(unsigned int);
    };

    class TCP : public CommonSocket
    {
    private:
        Address                               _address;
        std::vector<std::pair<int, Address> > _clients;
#ifdef WINDOWS
        HANDLE                                _clients_mutex;
#else
        pthread_mutex_t                       _clients_mutex;
#endif
    public:
        TCP(void);
        TCP(const TCP&);
        ~TCP(void);
        void close(void);

        Ip get_ip(void);
        Port get_port(void);
        Address get_address(void);

        void listen_on_port(Port, unsigned int listeners = 1);
        void connect_to(Address);

        TCP accept_client(void);

        template <class T> int send(const T*, size_t);
        template <class T> int receive(T*, size_t);

        template <class T> int send_timeout(unsigned int, const T*, size_t);
        template <class T> int receive_timeout(unsigned int, T*, size_t);

        void send_file(std::string);
        void receive_file(std::string);

        int accept_all(TCP&) throw();
        template <class T> int select_receive_all(TCP&, T*, size_t) throw();
    };
}

#include "Address.cpp"
#include "CommonSocket.cpp"
#include "Datagram.cpp"
#include "SocketException.cpp"
#include "TCP.cpp"
#include "UDP.cpp"

#endif
