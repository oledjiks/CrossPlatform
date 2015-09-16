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
#include <string.h>
#include <fcntl.h>
#endif

#include <iostream>
#include <sstream>
#include <exception>
#include <string>
#include <vector>
#include <fstream>

#define SOCKET_TIMEOUT            (0)
#ifndef WINDOWS
#define SOCKET_ERROR              (-1)
#endif
#define SOCKET_CLOSE              (-2)

#ifndef SOCKET_MAX_BUFFER_BYTES
#define SOCKET_MAX_BUFFER_BYTES   (64 << 10) // 64KB
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

        inline Ip get_ip(void) const;
        inline Ip set_ip(Ip);

        inline Port get_port(void) const;
        inline Port set_port(Port);

        friend std::ostream& operator<< (std::ostream&, Address&);
    };

    template <typename DataType>
    struct Datagram
    {
    public:
        Address             address;
        DataType            data;
        unsigned int        received_bytes;
        unsigned int        received_elements;

        Datagram();

        template <typename T> void operator= (const Datagram<T>&);
#if __cplusplus >= 201103L
        template <typename T> void operator= (Datagram<T>&&);
#endif
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

        inline SocketId get_socket_id(void);

        void open(void);
        void close(void);

        virtual void bind_on_port(Port);
        int set_option(int, int, const void*, socklen_t);
        int get_option(int, int, void*, socklen_t*);

        // Set option common
        int set_broadcast(bool isboardcast);
        int set_nonblock(bool isnonblock);
        int set_ttl(int ttl);
        int get_ttl(int& ttl);
        int set_multicast_ttl(int ttl);
        int set_tos(int tos);
        int get_tos(int& tos);
        int set_timeout(int sendtimeout, int recvtimeout);
        int get_timeout(int& sendtimeout, int& recvtimeout);
        int set_buffsize(int sendbuffsize, int recvbuffsize);
        int get_buffsize(int& sendbuffsize, int& recvbuffsize);
        int set_dontfragment(bool isdf);
    };

    class UDP : public CommonSocket
    {
    public:
        UDP(void);
        UDP(const UDP&);

    public:
        int send(Ip ip, Port port, const char* data, size_t len);
        int send(const Address& address, const char* data, size_t len);

        template <typename T> int send(Ip, Port, const T*, size_t);
        template <typename T> int send(const Address&, const T*, size_t);
        template <typename T> int send(Ip, Port, T);
        template <typename T> int send(const Address&, T);

    public:
        int receive(Address& address, char* data, unsigned int& len);
        int receive_timeout(unsigned int ms, Address& address, char* data, unsigned int& len);

        template <typename T> inline int receive(Address&, T*, size_t, unsigned int&);
        template <typename T> inline int receive_timeout(unsigned int, Address&, T*, size_t, unsigned int&);

    public:
        template <typename T> Datagram<T*> receive(T*, size_t len = SOCKET_MAX_BUFFER_BYTES / sizeof(T));
        template <typename T, size_t N> Datagram<T[N]> receive(size_t len = N);
        template <typename T> Datagram<T> receive(void);

        template <typename T> Datagram<T*> receive_timeout(unsigned int, T*, size_t len = SOCKET_MAX_BUFFER_BYTES / sizeof(T));
        template <typename T, size_t N> Datagram<T[N]> receive_timeout(unsigned int, size_t len = N);
        template <typename T> Datagram<T> receive_timeout(unsigned int);
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
        class LockGuard;        // Nested Classes
    public:
        TCP(void);
        TCP(const TCP&);
        ~TCP(void);

        // Set option
        int set_reuseaddr(bool isreuseaddr);
        int set_lingeroff();
        int set_lingeron(short timeoutsec);
        int set_nodelay(bool isnodelay);

        void close(void);

        Ip get_ip(void);
        Port get_port(void);
        Address get_address(void);

        void listen_on_port(Port, unsigned int listeners = 1);
        void connect_to(Address);

        TCP accept_client(void);

    public:
        int send(const char* buffer, size_t len);
        int receive(char*, size_t len);

        template <typename T> int send(const T*, size_t);
        template <typename T> int receive(T*, size_t);

        template <typename T> int send_timeout(unsigned int, const T*, size_t);
        template <typename T> int receive_timeout(unsigned int, T*, size_t);

        int accept_all(TCP&) throw();
        template <typename T> int receive_all(TCP&, unsigned int, T*, size_t) throw();
    };
}

#include "Address.cpp"
#include "CommonSocket.cpp"
#include "Datagram.cpp"
#include "SocketException.cpp"
#include "TCP.cpp"
#include "UDP.cpp"

#endif
