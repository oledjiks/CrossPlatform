#include "../Socket.h"
#include <string>

using namespace std;

#define IP   "127.0.0.1"
// #define IP   "10.14.5.107"
#define PORT 12345

int main(void)
{
    try
    {
        Socket::TCP client;
        client.connect_to(Socket::Address(IP, PORT));

        // cout << "sending ..." << endl;
        // client.send_file("input.bmp");

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
        cout << e << endl;
    }

    return 0;
}
