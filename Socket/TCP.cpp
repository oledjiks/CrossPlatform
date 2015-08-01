#ifndef _TCP_CPP_
#define _TCP_CPP_

#include "Socket.h"

namespace Socket
{
    TCP::TCP(void) : CommonSocket(SOCK_STREAM)
    {
        this->_clients = std::vector<std::pair<int, Address> >();
    }

    TCP::TCP(const TCP &tcp) : CommonSocket()
    {
        this->_socket_id = tcp._socket_id;
        this->_opened = tcp._opened;
        this->_binded = tcp._binded;
        this->_socket_type = tcp._socket_type;
        this->_address = tcp._address;
        this->_clients = tcp._clients;
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
        }

        this->_opened = false;
        this->_binded = false;
    }

    Ip TCP::ip(void)
    {
        return this->_address.ip();
    }

    Port TCP::port(void)
    {
        return this->_address.port();
    }

    Address TCP::address(void)
    {
        return Address(this->_address);
    }

    void TCP::listen_on_port(Port port, unsigned int listeners)
    {
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
        if (this->_binded) throw SocketException("[connect_to] Socket already binded to a port, use another socket");

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

        ret.close();
        ret._socket_id = accept(this->_socket_id, (struct sockaddr*)&ret._address, (socklen_t *)&len);
        ret._opened = true;
        ret._binded = true;

        return ret;
    }

    template <class T>
    int TCP::send(const T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[send] Socket not binded");
        if (!this->_opened) throw SocketException("[send] Socket not opened");

        len *= sizeof(T);
        if (len > (SOCKET_MAX_BUFFER_LEN * sizeof(T)))
        {
            std::stringstream error;
            error << "[send] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_LEN << ")";
            throw SocketException(error.str());
        }

        int ret;
        if ((ret = ::send(this->_socket_id, (const char*)buffer, len, 0)) == -1)
            throw SocketException("[send] Cannot send");
        return ret;
    }

    template <class T>
    int TCP::receive(T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[send_file] Socket not binded");
        if (!this->_opened) throw SocketException("[send_file] Socket not opened");

        len *= sizeof(T);
        if (len > (SOCKET_MAX_BUFFER_LEN * sizeof(T)))
        {
            std::stringstream error;
            error << "[receive] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_LEN << ")";
            throw SocketException(error.str());
        }

        int ret;
        if ((ret = ::recv(this->_socket_id, (char *)buffer, len, 0)) == -1)
            throw SocketException("[receive] Cannot receive");
        return ret;
    }

    template <class T>
    int TCP::send_timeout(unsigned int sec, const T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[send] Socket not binded");
        if (!this->_opened) throw SocketException("[send] Socket not opened");

        len *= sizeof(T);
        if (len > (SOCKET_MAX_BUFFER_LEN * sizeof(T)))
        {
            std::stringstream error;
            error << "[send] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_LEN << ")";
            throw SocketException(error.str());
        }

        int ret = -1;
        int ready;
        fd_set wset;
        struct timeval timeout = {(time_t)sec, 0};

        FD_ZERO(&wset);
        FD_SET(this->_socket_id, &wset);

        ready = ::select(this->_socket_id+1, NULL, &wset, NULL, &timeout);
        if (ready == SOCKET_ERROR)
        {
            throw SocketException("[send_timeout] select() return SOCKET_ERROR");
        }
        else if (ready > 0)
        {
            if (FD_ISSET(this->_socket_id, &wset))
            {
                if ((ret = ::send(buffer, len)) == -1)
                    throw SocketException("[send_timeout] Cannot send");
            }
        }

        return ret;
    }

    template <class T>
    int TCP::receive_timeout(unsigned int sec, T* buffer, size_t len)
    {
        if (!this->_binded) throw SocketException("[send] Socket not binded");
        if (!this->_opened) throw SocketException("[send] Socket not opened");

        len *= sizeof(T);
        if (len > (SOCKET_MAX_BUFFER_LEN * sizeof(T)))
        {
            std::stringstream error;
            error << "[send] [len=" << len << "] Data length higher then max buffer len ("
                  << SOCKET_MAX_BUFFER_LEN << ")";
            throw SocketException(error.str());
        }

        int ret = -1;
        int ready;
        fd_set rset;
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
            if (FD_ISSET(this->_socket_id, &rset))
            {
                if ((ret = ::recv(this->_socket_id, (char *)buffer, len, 0)) == -1)
                    throw SocketException("[receive_timeout] Cannot receive");
            }
        }

        return ret;
    }

    void TCP::send_file(std::string file_name)
    {
        unsigned long long file_size;
        char chunk[SOCKET_MAX_BUFFER_LEN];
        char sync;
        std::fstream fp(file_name.c_str(), std::ios::in | std::ios::binary);

        if (!fp.is_open())
        {
            std::stringstream error;
            error << "[send_file] with [filename=" << file_name << "] Cannot open the file";
            throw SocketException(error.str());
        }

        fp.seekg(0, std::ifstream::end);
        file_size = fp.tellg();
        fp.seekg(0, std::ifstream::beg);
        this->send<unsigned long long>(&file_size, 1);

        for (unsigned long long i = 0; i < file_size / SOCKET_MAX_BUFFER_LEN; i++)
        {
            this->receive<char>(&sync, 1);
            fp.read(chunk, SOCKET_MAX_BUFFER_LEN);
            this->send<char>(chunk, SOCKET_MAX_BUFFER_LEN);
        }

        if ((file_size % SOCKET_MAX_BUFFER_LEN) != 0)
        {
            this->receive<char>(&sync, 1);
            fp.read(chunk, file_size % SOCKET_MAX_BUFFER_LEN);
            this->send<char>(chunk, file_size % SOCKET_MAX_BUFFER_LEN);
        }

        fp.close();
    }

    void TCP::receive_file(std::string file_name)
    {
        unsigned long long file_size;
        char chunk[SOCKET_MAX_BUFFER_LEN];
        char sync;
        std::fstream fp(file_name.c_str(), std::ios::out | std::ios::binary);

        if (!fp.is_open())
        {
            std::stringstream error;
            error << "[receive_file] with [filename=" << file_name << "] Cannot open/create the file";
            throw SocketException(error.str());
        }

        this->receive<unsigned long long>(&file_size, 1);

        for(unsigned long long i = 0; i < file_size / SOCKET_MAX_BUFFER_LEN; i++)
        {
            this->send<char>(&sync, 1);
            this->receive<char>(chunk, SOCKET_MAX_BUFFER_LEN);
            fp.write(chunk, SOCKET_MAX_BUFFER_LEN);
        }

        if ((file_size % SOCKET_MAX_BUFFER_LEN) != 0)
        {
            this->send<char>(&sync, 1);
            this->receive<char>(chunk, file_size % SOCKET_MAX_BUFFER_LEN);
            fp.write(chunk, file_size % SOCKET_MAX_BUFFER_LEN);
        }

        fp.close();
    }

    int TCP::accept_all(void) throw()
    {
        SocketId client_id;
        Address client_address;
        socklen_t len = sizeof(struct sockaddr_in);

        if (this->_clients.size() >= FD_SETSIZE)
            return SOCKET_ERROR;

        client_id = accept(this->_socket_id, (struct sockaddr*)&client_address, (socklen_t *)&len);

        {
            // TODO: thread safe
            this->_clients.push_back(std::make_pair(client_id, client_address));
        }

#ifdef _DEBUG
        std::stringstream ss;
        ss << "in accept_client() clients number is: " << this->_clients.size()
           << "\t accepted client: " << client_address.ip() << ":" << client_address.port() << std::endl;
        std::cout << ss.str();
#endif
        return client_id;
    }

    template <class T>
    int TCP::select_receive_all(SocketId* client_id, Address* from, T* buffer, size_t len) throw()
    {
        int ready;
        int ret;
        SocketId maxfd = 0;
        fd_set client_rset;
        struct timeval tv = {0, 10000};

#ifdef _DEBUG
        std::stringstream ss;
#endif

        if (!this->_binded)
            return SOCKET_ERROR;
        if (!this->_opened)
            return SOCKET_ERROR;

        while (1)
        {
            size_t clients_num; // for thread safe (clients_num should be less then this->_clients_num)
            {
                // TODO: thread safe
                clients_num = this->_clients.size();
                FD_ZERO(&client_rset);
                for (unsigned int i = 0; i < clients_num; ++i)
                {
                    maxfd = (maxfd < this->_clients[i].first) ? this->_clients[i].first : maxfd;
                    FD_SET(this->_clients[i].first, &client_rset);
                }
            }

            ready = ::select(maxfd+1, &client_rset, NULL, NULL, &tv);

            // select() error
            if (ready == SOCKET_ERROR)
                return SOCKET_ERROR;

            // timeout
            if (ready == 0)
                continue;

            // something to read
#ifdef _DEBUG
            ss << "select() return: " << ready << std::endl;
            std::cout << ss.str();
#endif
            for (unsigned int i = 0; i < clients_num; ++i)
            {
                if (FD_ISSET(this->_clients[i].first, &client_rset))
                {
                    *client_id = this->_clients[i].first;
                    *from = this->_clients[i].second;
                    ret = ::recv(this->_clients[i].first, (char *)buffer, len, 0);
#ifdef _DEBUG
                    ss << "recvfrom() return: " << ret << std::endl;
                    std::cout << ss.str();
#endif
                    if (ret == 0 || (ret == SOCKET_ERROR
#ifdef WINDOWS
                                     && WSAGetLastError() == WSAECONNRESET
#else
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
                            // TODO: thread safe
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
}

#endif
