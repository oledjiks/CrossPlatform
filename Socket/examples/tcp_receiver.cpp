#include "../Socket.h"

using namespace std;

#define IP   "127.0.0.1"
// #define IP   "10.14.5.107"
#define PORT 12345

int main(void)
{
    try
    {
        Socket::TCP server;

        server.listen_on_port(PORT);
        Socket::TCP client = server.accept_client();

        cout << "receiving ..." << endl;
        client.receive_file("output.bmp");
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
