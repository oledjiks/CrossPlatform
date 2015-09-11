#ifndef _TCP_CPP_
#define _TCP_CPP_

#include "Socket.h"

#ifdef WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

namespace Socket
{
    class TCP::LockGuard
    {
    private:
#ifdef WINDOWS
        HANDLE&          _mutex;
#else
        pthread_mutex_t& _mutex;
#endif

    public:
#ifdef WINDOWS
        LockGuard(HANDLE& m) : _mutex(m)
        {
            if (WaitForSingleObject(this->_mutex, INFINITE) != WAIT_OBJECT_0)
                throw std::exception();
        }
#else
        LockGuard(pthread_mutex_t& m) : _mutex(m)
        {
            if (pthread_mutex_lock(&this->_mutex))
                throw std::exception();
        }
#endif

#ifdef WINDOWS
        virtual ~LockGuard()
        {
            if (!ReleaseMutex(this->_mutex))
                throw std::exception();
        }
#else
        virtual ~LockGuard()
        {
            if (pthread_mutex_unlock(&this->_mutex))
                throw std::exception();
        }
#endif
    };

    TCP::TCP(void) : CommonSocket(SOCK_STREAM)
    {
        this->_clients = std::vector<std::pair<int, Address> >();
#ifdef WINDOWS
        this->_clients_mutex = CreateMutex(NULL, false, NULL);
#else
        pthread_mutex_init(&this->_clients_mutex, NULL);
#endif
    }

    TCP::TCP(const TCP &tcp) : CommonSocket()
    {
        this->_socket_id = tcp._socket_id;
        this->_opened = tcp._opened;
        this->_binded = tcp._binded;
        this->_socket_type = tcp._socket_type;
        this->_address = tcp._address;
        this->_clients = tcp._clients;
#ifdef WINDOWS
        this->_clients_mutex = CreateMutex(NULL, false, NULL);
#else
        pthread_mutex_init(&this->_clients_mutex, NULL);
#endif
    }

    TCP::~TCP(void)
    {
#ifdef WINDOWS
        CloseHandle(this->_clients_mutex);
#else
        pthread_mutex_destroy(&this->_clients_mutex);
#endif
    }

    int TCP::set_lingeroff()
    {
        int ret = 0;
        linger socklinger;
        socklinger.l_onoff  = 0;
        socklinger.l_linger = 0;
        if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_LINGER, (char*)&socklinger,sizeof(linger))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_lingeroff] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int TCP::set_lingeron(short timeoutsec)
    {
        int ret = 0;
        linger socklinger;
        socklinger.l_onoff  = 1;
        socklinger.l_linger = timeoutsec;
        if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_LINGER, (char*)&socklinger,sizeof(linger))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_lingeron] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int TCP::set_reuseaddr(bool isreuseaddr)
    {
        int ret = 0;
        if ((ret = setsockopt(_socket_id, SOL_SOCKET, SO_REUSEADDR, (char*)&isreuseaddr, sizeof(bool))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_reuseaddr] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    int TCP::set_nodelay(bool isnodelay)
    {
        int ret = 0;
        if ((ret = setsockopt(_socket_id, IPPROTO_TCP, TCP_NODELAY, (const char*)&isnodelay, sizeof(bool))) == SOCKET_ERROR)
        {
            std::stringstream error;
            error << "[set_nodelay] error";
            throw SocketException(error.str());
        }
        return ret;
    }

    void TCP::close(void)
    {
        if (this->_opened)
        {
#ifdef WINDOWS
            closesocket(this->_socket_id);
#else
            shutdown(this->_socket_id, SHUT_RDWR);
            ::close(this->_socket_id);
#endif
            for (unsigned int i = 0; i < this->_clients.size(); ++i)
            {
#ifdef WINDOWS
                closesocket(this->_clients[i].first);
#else
                shutdown(this->_clients[i].first, SHUT_RDWR);
                ::close(this->_clients[i].first);
#endif
            }
            this->_clients.clear();
        }

        this->_opened = false;
        this->_binded = false;
    }

    Ip TCP::get_ip(void)
    {
        return this->_address.get_ip();
    }

    Port TCP::get_port(void)
    {
        return this->_address.get_port();
    }

    Address TCP::get_address(void)
    {
        return Address(this->_address);
    }

    void TCP::listen_on_port(Port port, unsigned int listeners)
    {
        if (this->_binded)
        {
            if (this->_address.get_port() != port)
                throw SocketException("[listen_on_port] Socket listen to a port different from binded");
        }
        else
            CommonSocket::bind_on_port(port);

        if (listen(this->_socket_id, listeners) != 0)
        {
            std::stringstream error;
            error << "[listen_on_port] with [port=" << port << "] [listeners=" << listeners << "] Cannot listen";
            throw SocketException(error.str());
        }

        this->_clients.resize(FD_SETSIZE);
        this->_clients.clear();
    }

    void TCP::connect_to(Address address)
    {
        if (!this->_opened) this->open();

        if (connect(this->_socket_id, (struct sockaddr*)&address, sizeof(struct sockaddr_in)) < 0)
        {
            std::stringstream error;
            error << "[connect_to] with [address=" << address << "] Cannot connect to the specified address";
            throw SocketException(error.str());
        }

        this->_binded = true;
    }

    TCP TCP::accept_client(void)
    {
        TCP ret;
        socklen_t len = sizeof(struct sockaddr_in);

        ret._socket_id = accept(this->_socket_id, (struct sockaddr*)&ret._address, (socklen_t *)&len);
        ret._opened = true;
        ret._binded = true;

        return ret;
    }

    template <typename T>
    int TCP::send(const T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[send] Socket not binded");
        if (!this->_opened) throw SocketException("[send] Socket not opened");

        len *= sizeof(T);
        if (len > SOCKET_MAX_BUFFER_BYTES)
        {
            std::stringstream error;
            error << "[send] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_BYTES << ")";
            throw SocketException(error.str());
        }

        int ret;
        if ((ret = ::send(this->_socket_id, (const char*)buffer, len, 0)) == SOCKET_ERROR)
            throw SocketException("[send] Cannot send");
        return ret;
    }

    template <typename T>
    int TCP::receive(T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[receive] Socket not binded");
        if (!this->_opened) throw SocketException("[receive] Socket not opened");

        len *= sizeof(T);
        if (len > SOCKET_MAX_BUFFER_BYTES)
        {
            std::stringstream error;
            error << "[receive] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_BYTES << ")";
            throw SocketException(error.str());
        }

        int ret;
        ret = ::recv(this->_socket_id, (char *)buffer, len, 0);
        if (ret == 0 || (ret == SOCKET_ERROR
#ifdef WINDOWS
                         && WSAGetLastError() == WSAECONNRESET
#else
                         && errno == ECONNRESET
#endif
                ))
        {
            ret = SOCKET_CLOSE;
        }

        return ret;
    }

    template <typename T>
    int TCP::send_timeout(unsigned int ms, const T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[send_timeout] Socket not binded");
        if (!this->_opened) throw SocketException("[send_timeout] Socket not opened");

        len *= sizeof(T);
        if (len > SOCKET_MAX_BUFFER_BYTES)
        {
            std::stringstream error;
            error << "[send_timeout] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_BYTES << ")";
            throw SocketException(error.str());
        }

        int ret = SOCKET_ERROR;
        int ready;
        fd_set wset;
        struct timeval timeout = {(time_t)(ms/1000), ms%1000};

        FD_ZERO(&wset);
        FD_SET(this->_socket_id, &wset);

        ready = ::select(this->_socket_id+1, NULL, &wset, NULL, &timeout);
        // error
        if (ready == SOCKET_ERROR || ready < 0)
        {
            throw SocketException("[send_timeout] select() return SOCKET_ERROR");
        }

        // timeout
        if (ready == 0)
            return 0;

        // something to write
        if (FD_ISSET(this->_socket_id, &wset))
        {
            if ((ret = ::send(this->_socket_id, (const char*)buffer, len, 0)) == SOCKET_ERROR)
                throw SocketException("[send_timeout] Cannot send");
        }

        return ret;
    }

    template <typename T>
    int TCP::receive_timeout(unsigned int ms, T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[receive_timeout] Socket not binded");
        if (!this->_opened) throw SocketException("[receive_timeout] Socket not opened");

        len *= sizeof(T);
        if (len > SOCKET_MAX_BUFFER_BYTES)
        {
            std::stringstream error;
            error << "[receive_timeout] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_BYTES << ")";
            throw SocketException(error.str());
        }

        int ret = SOCKET_ERROR;
        int ready;
        fd_set rset;
        struct timeval timeout = {(time_t)(ms/1000), ms%1000};

        FD_ZERO(&rset);
        FD_SET(this->_socket_id, &rset);

        ready = ::select(this->_socket_id+1, &rset, NULL, NULL, &timeout);
        // error
        if (ready == SOCKET_ERROR || ready < 0)
        {
            throw SocketException("[receive_timeout] select() return SOCKET_ERROR");
        }

        // timeout
        if (ready == 0)
            return SOCKET_TIMEOUT;

        // something to read
        if (FD_ISSET(this->_socket_id, &rset))
        {
            ret = ::recv(this->_socket_id, (char *)buffer, len, 0);
            if (ret == 0 || (ret == SOCKET_ERROR
#ifdef WINDOWS
                             && WSAGetLastError() == WSAECONNRESET
#else
                             && errno == ECONNRESET
#endif
                    ))
            {
                ret = SOCKET_CLOSE;
            }
        }

        return ret;
    }

    int TCP::accept_all(TCP& client) throw()
    {
        socklen_t len = sizeof(struct sockaddr_in);

        if (this->_clients.size() >= FD_SETSIZE)
            return SOCKET_ERROR;

        client._socket_id = accept(this->_socket_id, (struct sockaddr*)&client._address, (socklen_t *)&len);

        {
            LockGuard lock(this->_clients_mutex);
            this->_clients.push_back(std::make_pair(client._socket_id, client._address));
        }

#ifdef _DEBUG
        std::stringstream ss;
        ss << "in accept_client() clients number is: " << this->_clients.size()
           << "\t accepted client: " << client._address.get_ip() << ":" << client._address.get_port() << std::endl;
        std::cout << ss.str();
#endif
        return client._socket_id;
    }

    template <typename T>
    int TCP::receive_all(TCP& client, unsigned int ms, T* buffer, size_t len) throw()
    {
        int ready;
        int ret;
        SocketId maxfd = 0;
        fd_set client_rset;
        struct timeval tv = {(time_t)(ms/1000), ms%1000};

#ifdef _DEBUG
        std::stringstream ss;
#endif

        if (!this->_binded)
            return SOCKET_ERROR;
        if (!this->_opened)
            return SOCKET_ERROR;

        size_t clients_num; // for thread safe
        {
            LockGuard lock(this->_clients_mutex);
            clients_num = this->_clients.size();
            FD_ZERO(&client_rset);
            for (unsigned int i = 0; i < clients_num; ++i)
            {
                maxfd = (maxfd < this->_clients[i].first) ? this->_clients[i].first : maxfd;
                FD_SET(this->_clients[i].first, &client_rset);
            }
        }

        ready = ::select(maxfd+1, &client_rset, NULL, NULL, &tv);

        // error
        if (ready == SOCKET_ERROR || ready < 0)
            return SOCKET_ERROR;

        // timeout
        if (ready == 0)
            return SOCKET_TIMEOUT;

        // something to read
#ifdef _DEBUG
        ss << "select() return: " << ready << std::endl;
        std::cout << ss.str();
#endif
        for (unsigned int i = 0; i < clients_num; ++i)
        {
            if (FD_ISSET(this->_clients[i].first, &client_rset))
            {
                client._socket_id = this->_clients[i].first;
                client._address = this->_clients[i].second;
                ret = ::recv(this->_clients[i].first, (char *)buffer, len, 0);
#ifdef _DEBUG
                ss << "recvfrom() return: " << ret << std::endl;
                std::cout << ss.str();
#endif
                if (ret == 0 || (ret == SOCKET_ERROR
#ifdef WINDOWS
                                 && WSAGetLastError() == WSAECONNRESET
#else
                                 && errno == ECONNRESET
#endif
                        ))
                {
                    // Client socket closed
#ifdef WINDOWS
                    closesocket(this->_clients[i].first);
#else
                    shutdown(this->_clients[i].first, SHUT_RDWR);
                    ::close(this->_clients[i].first);
#endif

#ifdef _DEBUG
                    ss << "client (socket_id = " << this->_clients[i].first
                       << ", socket_address = " << this->_clients[i].second << ") closed" << std::endl;
                    std::cout << ss.str();
#endif
                    {
                        LockGuard lock(this->_clients_mutex);
                        this->_clients.erase(this->_clients.begin() + i);
                    }
#ifdef _DEBUG
                    ss << "in select_receive() clients number is: " << this->_clients.size() << std::endl;
                    std::cout << ss.str();
#endif
                    break;
                }
                else
                {
                    // Receive something from client
                }
            }
        }
        return ret;
    }
}

#endif
