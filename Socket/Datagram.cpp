#ifndef _DATAGRAM_CPP_
#define _DATAGRAM_CPP_

#include "Socket.h"

namespace Socket
{
template <typename DataType>
Datagram<DataType>::Datagram() : received_bytes(0), received_elements(0)
{
}

template <typename DataType>
template <typename T>
void Datagram<DataType>::operator= (const Datagram<T> &datagram)
{
    this->address = datagram.address;
    this->data = datagram.data;
}

#if __cplusplus >= 201103L
template <typename DataType>
template <typename T>
void Datagram<DataType>::operator= (Datagram<T>&& datagram)
{
    this->address = datagram.address;
    this->data = std::move(datagram.data);
}
#endif
}

#endif
