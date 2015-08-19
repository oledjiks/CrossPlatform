#ifndef _SOCKETEXCEPTION_CPP_
#define _SOCKETEXCEPTION_CPP_

#include "Socket.h"

namespace Socket
{
    SocketException::SocketException(const std::string &message)
    {
        std::stringstream error;
        error << message;
#ifdef WINDOWS
        unsigned long dw = GetLastError();
        LPVOID lpmsg;
        char sz[128];
        size_t len = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                    // MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    (LPSTR)&lpmsg, 0, NULL);
        sprintf(sz, "%s", lpmsg);
        if (len > 2)
            sz[len - 2] = '\0';
        else
            sz[0] = '\0';
        LocalFree(lpmsg);
        error << " (WSAGetLastError=" << dw << ", " << sz << ")";
#else
        error << " (errno=" << errno << ", " << strerror(errno) << ")";
#endif
        this->_error = error.str();
    }

    SocketException::~SocketException() throw()
    {
    }

    const char* SocketException::what() const throw()
    {
        return this->_error.c_str();
    }

    std::ostream& operator<< (std::ostream &out, SocketException &e)
    {
        out << e.what();
        return out;
    }
}

#endif
