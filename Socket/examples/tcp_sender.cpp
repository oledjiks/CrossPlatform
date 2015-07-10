#include "../Socket.h"

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

        cout << "sending ..." << endl;
        client.send_file("input.bmp");
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
