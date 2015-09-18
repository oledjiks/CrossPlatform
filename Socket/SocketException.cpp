#ifndef _SOCKETEXCEPTION_CPP_
#define _SOCKETEXCEPTION_CPP_

#include "Socket.h"

namespace Socket
{
    SocketException::SocketException(const std::string &message)
    {
        this->_error = message;
    }

    SocketException::~SocketException() throw()
    {
    }

    const char* SocketException::what() const throw()
    {
        return this->_error.c_str();
    }

    unsigned long SocketException::get_error(std::string& err_msg)
    {
#ifdef WINDOWS
        unsigned long dw = WSAGetLastError();
        char sz[128];
        LPVOID lpmsg;
        size_t len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                    // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    (LPSTR)&lpmsg, 0, NULL);
        sprintf_s(sz, len+1, "%s", lpmsg);
        if (len > 2)
            sz[len - 2] = '\0';
        else
            sz[len] = '\0';
        LocalFree(lpmsg);
        err_msg = sz;
#else
        unsigned long dw = errno;
        err_msg = strerror(errno);
#endif

        return dw;
    }

    std::ostream& operator<< (std::ostream &out, SocketException &e)
    {
        out << e.what();
        return out;
    }
}

#endif
