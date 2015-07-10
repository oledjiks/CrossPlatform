#ifndef _SOCKETEXCEPTION_CPP_
#define _SOCKETEXCEPTION_CPP_

#include "Socket.h"

namespace Socket
{
    SocketException::SocketException(const string &message)
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

    ostream& operator<< (ostream &out, SocketException &e)
    {
        out << e.what();
        return out;
    }
}

#endif
