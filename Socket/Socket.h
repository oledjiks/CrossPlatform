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
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define SOCKET_MAX_BUFFER_LEN   1024
#ifndef WINDOWS
#define SOCKET_ERROR			(-1)
#endif

namespace Socket
{
    typedef int SocketId;
    typedef std::string Ip;
    typedef unsigned int Port;
#ifdef WINDOWS
    typedef unsigned int socklen_t;
#endif

    class SocketException : public std::exception
    {
    private:
        std::string _error;

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

        Ip ip(void);
        Ip ip(Ip);

        Port port(void);
        Port port(Port);

        friend std::ostream& operator<< (std::ostream&, Address&);
    };

    template <class DataType>
    struct Datagram
    {
    public:
        Address address;
        DataType data;
        unsigned int received_bytes;
        unsigned int received_elements;

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
        SocketId _socket_id;
        int _socket_type;
        bool _opened;
        bool _binded;

    public:
        CommonSocket(void);
        CommonSocket(int);

        ~CommonSocket(void);

        void open(void);
        void close(void);

        virtual void listen_on_port(Port);
        int set_option(int, int, const char *, socklen_t);
    };

    class UDP : public CommonSocket
    {
    public:
        UDP(void);
        UDP(const UDP&);

        template <class T> int send(Ip, Port, const T*, size_t);
        template <class T> int send(Address, const T*, size_t);
        template <class T> int send(Ip, Port, T);
        template <class T> int send(Address, T);
        template <class T> int send(Ip, Port, std::vector<T>);
        template <class T> int send(Address, std::vector<T>);

    protected:
        template <class T> int receive(Address*, T*, size_t, unsigned int*);

    public:
        template <class T> Datagram<T*> receive(T*, size_t len = SOCKET_MAX_BUFFER_LEN);
        template <class T, size_t N> Datagram<T[N]> receive(size_t len = N);
        template <class T> Datagram<T> receive(void);
        template <class T> Datagram<std::vector<T> > receive(size_t);
    };

    class TCP : public CommonSocket
    {
    private:
        Address _address;
    public:
        TCP(void);
        TCP(const TCP&);

        Ip ip(void);
        Port port(void);
        Address address(void);

        void listen_on_port(Port, unsigned int listeners = 1);
        void connect_to(Address);

        TCP accept_client(void);

        template <class T> int send(const T*, size_t);
        template <class T> int receive(T*, size_t);

        void send_file(std::string);
        void receive_file(std::string);
    };
}

#include "Address.cpp"
#include "CommonSocket.cpp"
#include "Datagram.cpp"
#include "SocketException.cpp"
#include "TCP.cpp"
#include "UDP.cpp"

#endif
