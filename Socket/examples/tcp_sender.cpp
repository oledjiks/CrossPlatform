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
        {
            // Simple prototype
            Socket::TCP client;
            client.open();

            client.connect_to(Socket::Address(IP, PORT));

            cout << "sending ..." << endl;
            client.send_file("input.bmp");
            client.close();
            cout << "Client1 close\n";
        }

        cout << endl
             << "-------- Multi I/O prototype --------" << endl
             << "Any key to continue ...";
        getchar();
        {
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
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
