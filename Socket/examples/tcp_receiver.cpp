#include "../Socket.h"
#include <thread>

using namespace std;

#define IP   "127.0.0.1"
// #define IP   "10.14.5.107"
#define PORT 12345

void receiving_all_msg(Socket::TCP& server)
{
    int len;
    Socket::SocketId client_id;
    Socket::Address from;
    char buffer[SOCKET_MAX_BUFFER_LEN];

    while (1)
    {
        len = server.select_receive_all<char>(&client_id, &from, buffer, SOCKET_MAX_BUFFER_LEN-1);
        if (len > 0)
        {
            buffer[len] = '\0';
            cout << "--> [TCP] client (" << client_id
                 << ")"<< from.ip() << ":" << from.port()
                 << " <" << len << "> " << buffer << endl;
        }
    }
}

void accepting_all_client(Socket::TCP& server)
{
    while (1)
    {
        if (server.accept_all() == SOCKET_ERROR)
            break;
    }
}

int main(void)
{
    try
    {
        {
            // Simple prototype
            Socket::TCP server;

            server.listen_on_port(PORT);
            Socket::TCP client = server.accept_client();

            cout << "receiving ..." << endl;
            client.receive_file("output.bmp");
            server.close();
            cout << "Server1 close\n";
        }

        {
            // Multi I/O prototype
            Socket::TCP server;

            server.listen_on_port(PORT);

            thread accepting_thread(accepting_all_client, ref(server));
            thread receiving_thread(receiving_all_msg, ref(server));
            accepting_thread.join();
            receiving_thread.join();
            server.close();
        }
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
