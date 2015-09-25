#include "../Socket.h"
#include <iostream>
#include <string>

using namespace std;

#define IP   "127.0.0.1"
// #define IP   "10.14.5.107"
#define PORT 12345

int main(void)
{
    try
    {
        cout << "-------- Simple prototype --------" << endl;
        // Simple prototype
        Socket::TCP client;
        client.open();
        client.bind_on_port(PORT+2);
        client.connect_to(Socket::Address(IP, PORT));

        string str_buffer;
        cout << "input message to send: ";
        cin >> str_buffer;
        client.send_timeout<char>(1000, str_buffer.c_str(), str_buffer.length());
        // client.send<char>(str_buffer.c_str(), str_buffer.length());

        client.close();
        cout << "Client close\n";
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    try
    {
        cout << endl
             << "-------- Multi I/O prototype --------" << endl
             << "Any key to continue ...";
        getchar();
        getchar();
        // Multi I/O prototype
        Socket::TCP client;
        client.open();

        client.connect_to(Socket::Address(IP, PORT));

        string str_buffer;
        while (1)
        {
            cout << "input message string: ";
            if (!(cin >> str_buffer))
                break;
            client.send(str_buffer.c_str(), str_buffer.length());
        }
        client.close();
    }
    catch (Socket::SocketException &e)
    {
        int lasterror;
        string err_msg;
        if ((lasterror = e.get_error(err_msg)) > 0)
            cout  << "lasterror = " << lasterror << ", " << err_msg << endl;
        cout << e << endl;
    }

    return 0;
}
