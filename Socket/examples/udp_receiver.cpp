#include "../Socket.h"

using namespace std;

struct prova
{
    int something;
    float somethingelse;
};

int main(void)
{
    try
    {
        Socket::UDP sock;
        sock.open();

        double buffer[SOCKET_MAX_BUFFER_BYTES / sizeof(double)];
        unsigned int i;
        int buffer_size = SOCKET_MAX_BUFFER_BYTES / sizeof(double);

        // sock.set_option(SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(char)); // throw ERROR
        sock.set_option(SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(buffer_size));
        sock.bind_on_port(10000);

        // Socket::Datagram<string>            rec_str = sock.receive<string>();
        Socket::Datagram<string>            rec_str;
        do
        {
            rec_str = sock.receive_timeout<string>(1000);
            if (rec_str.received_bytes > 0)
                break;
            else
                cout << "receive timeout" << endl;
        }
        while (1);
        cout << rec_str.data << endl;
        cout << endl;


        Socket::Datagram<int[5]>            rec_arr = sock.receive<int, 5>(); // ([, 5]);
        for (i = 0; i < 5; i++)
            cout << rec_arr.data[i] << " ";
        cout << endl << endl;


        Socket::Datagram<float>             rec_var = sock.receive<float>();
        cout << rec_var.data << endl;
        cout << endl;


        // (buffer [, SOCKET_MAX_BUFFER_BYTES]);
        Socket::Datagram<double*>           rec_pnt = sock.receive<double>(buffer);
        for (i = 0; i < rec_pnt.received_elements; i++)
            cout << rec_pnt.data[i] << " ";
        cout << endl;


        sock.close();
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
