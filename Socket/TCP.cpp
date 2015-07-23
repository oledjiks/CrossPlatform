#ifndef _TCP_CPP_
#define _TCP_CPP_

#include "Socket.h"

namespace Socket
{
    TCP::TCP(void) : CommonSocket(SOCK_STREAM)
    {
        this->_clients_num = 0;
        this->_clients_address = std::vector<Address>();
    }

    TCP::TCP(const TCP &tcp) : CommonSocket()
    {
        this->_socket_id = tcp._socket_id;
        this->_opened = tcp._opened;
        this->_binded = tcp._binded;
        this->_socket_type = tcp._socket_type;
        this->_address = tcp._address;
        this->_clients_num = tcp._clients_num;
        for (size_t i = 0; i < this->_clients_num; ++i)
            this->_clients[i] = tcp._clients[i];
        for (size_t i = 0; i < tcp._clients_address.size(); ++i)
            this->_clients_address[i] = tcp._clients_address[i];
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
            for (unsigned int i = 0; i < this->_clients_num; ++i)
            {
#ifdef WINDOWS
                closesocket(this->_clients[i]);
#else
                shutdown(this->_clients[i], SHUT_RDWR);
                ::close(this->_clients[i]);
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

        if (this->_clients_num > FD_SETSIZE)
            return ret;

        ret.close();
        ret._socket_id = accept(this->_socket_id, (struct sockaddr*)&ret._address, (socklen_t *)&len);
        ret._opened = true;
        ret._binded = true;

        return ret;
    }

    int TCP::accept_all(void)
    {
        SocketId client_id;
        Address client_address;
        socklen_t len = sizeof(struct sockaddr_in);

        if (this->_clients_num > FD_SETSIZE)
            return SOCKET_ERROR;

        client_id = accept(this->_socket_id, (struct sockaddr*)&client_address, (socklen_t *)&len);

        this->_clients[this->_clients_num] = client_id;
        ++(this->_clients_num);
        this->_clients_address.push_back(client_address);

#ifdef _DEBUG
        std::stringstream ss;
        ss << "in accept_client() _clients_num is: " << this->_clients_num
           << "\tclient: " << client_address.ip() << ":" << client_address.port() << std::endl;
        std::cout << ss.str();
#endif
        return client_id;
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

    template <class T>
    int TCP::select_receive_all(SocketId* client_id, Address* from, T* buffer, size_t len)
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
                clients_num = this->_clients_num;
                FD_ZERO(&client_rset);
                for (unsigned int i = 0; i < clients_num; ++i)
                {
                    maxfd = (maxfd < (this->_clients)[i]) ? (this->_clients)[i] : maxfd;
                    FD_SET((this->_clients)[i], &client_rset);
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
                if (FD_ISSET((this->_clients)[i], &client_rset))
                {
                    *client_id = (this->_clients)[i];
                    *from = this->_clients_address[i];
                    ret = ::recv((this->_clients)[i], (char *)buffer, len, 0);
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
                        closesocket((this->_clients)[i]);
#else
                        shutdown((this->_clients)[i], SHUT_RDWR);
#endif

#ifdef _DEBUG
                        ss << "client (" << (this->_clients)[i] << ") close" << std::endl;
                        std::cout << ss.str();
#endif
                        {
                            // TODO: thread safe
                            for (unsigned int j = i; j < this->_clients_num - 1; ++j)
                            {
                                (this->_clients)[j] = (this->_clients)[j+1];
                            }
                            --(this->_clients_num);
                            this->_clients_address.erase(this->_clients_address.begin() + i);
                        }
#ifdef _DEBUG
                        ss << "in select_receive() _clients_num is: " << this->_clients_num << std::endl;
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
