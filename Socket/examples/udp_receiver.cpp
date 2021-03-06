#include "../Socket.h"
#include <string>

using namespace std;

struct prova
{
    int something;
    float somethingelse;
};

int main(void)
{
    char* buffer = new char[SOCKET_MAX_BUFFER_BYTES];

    try
    {
        Socket::UDP sock;
        sock.open();

        unsigned int i;
        int buffer_size = SOCKET_MAX_BUFFER_BYTES / sizeof(double);

        // sock.set_option(SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(char)); // throw ERROR
        sock.set_option(SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(buffer_size));
		sock.set_dontfragment(true);
		sock.set_broadcast(true);
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
        Socket::Datagram<double*>           rec_pnt = sock.receive<double>((double*)buffer);
        for (i = 0; i < rec_pnt.received_elements; i++)
            cout << rec_pnt.data[i] << " ";
        cout << endl;


        Socket::Datagram<prova*>            rec_stct = sock.receive<prova>((prova*)buffer);
        cout << "struct prova: {" << rec_stct.data->something << ", " << rec_stct.data->somethingelse << "}" << endl;


        sock.close();
    }
    catch (Socket::SocketException &e)
    {
        int lasterror;
        string err_msg;
        if ((lasterror = e.get_error(err_msg)) > 0)
            cout  << "lasterror = " << lasterror << ", " << err_msg << endl;
        cout << e.what() << endl;
    }

    if (buffer)
    {
        delete[] buffer;
        buffer = nullptr;
    }

    return 0;
}
