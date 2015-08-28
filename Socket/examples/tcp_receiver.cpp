#include "../Socket.h"
#include <iostream>
#include <thread>

using namespace std;

#define IP   "127.0.0.1"
// #define IP   "10.14.5.107"
#define PORT 12345

void receiving_all_msg(Socket::TCP& server)
{
    int len;
    Socket::TCP client;
    char buffer[SOCKET_MAX_BUFFER_BYTES];

    while (1)
    {
        len = server.select_receive_all<char>(client, buffer, SOCKET_MAX_BUFFER_BYTES-1);
        if (len > 0)
        {
            buffer[len] = '\0';
            cout << "--> [TCP] client (" << client.get_socket_id()
                 << ")"<< client.get_ip() << ":" << client.get_port()
                 << " <" << len << "> " << buffer << endl;
        }
    }
}

void accepting_all_client(Socket::TCP& server)
{
    Socket::TCP client;

    while (1)
    {
        if (server.accept_all(client) == SOCKET_ERROR)
            break;
    }
}

int main(void)
{
    try
    {
        cout << "-------- Simple prototype --------" << endl;
        {
            // Simple prototype
            Socket::TCP server;
            server.open();

            server.listen_on_port(PORT);
            Socket::TCP client = server.accept_client();

            // cout << "receiving ..." << endl;
            // client.receive_file("output.bmp");

            char buff[100];
            int len;
            do
            {
                len = client.receive_timeout<char>(1000, buff, 100);
                if (len > 0)
                {
                    buff[len] = '\0';
                    cout << "received(" << len << "): " << buff << endl;
					break;
                }
                else
                {
                    cout << "receiveing timeout" << endl;
                }
            }
            while (1);

            server.close();
            cout << "Server1 close\n";
        }

        cout << endl
             << "-------- Multi I/O prototype --------" << endl;
        {
            // Multi I/O prototype
            Socket::TCP server;
            server.open();

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
