#ifndef _DATAGRAM_CPP_
#define _DATAGRAM_CPP_

#include "Socket.h"

namespace Socket
{
    template <class DataType>
    template <class T>
    void Datagram<DataType>::operator= (const Datagram<T> &datagram)
    {
        this->address = datagram.address;
        this->data = datagram.data;
    }
}

#endif
