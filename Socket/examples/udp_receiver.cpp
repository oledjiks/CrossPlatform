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
        double buffer[SOCKET_MAX_BUFFER_LEN];
        unsigned int i;
        int buffer_size = SOCKET_MAX_BUFFER_LEN;

        // sock.set_option(SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(char)); // throw ERROR
        sock.set_option(SOL_SOCKET, SO_SNDBUF, (const char*)&buffer_size, sizeof(buffer_size));
        sock.listen_on_port(10000);

        Socket::Datagram<string>            rec_str = sock.receive<string>();
        Socket::Datagram<int[5]>            rec_arr = sock.receive<int, 5>(); // ([, 5]);
        Socket::Datagram<float>             rec_var = sock.receive<float>();
        // (buffer [, SOCKET_MAX_BUFFER_LEN]);
        Socket::Datagram<double*>           rec_pnt = sock.receive<double>(buffer);
        // conflict with the first one, must be specified
        Socket::Datagram<vector<prova> >    rec_vec = sock.receive<prova>(5);

        cout << rec_str.data << endl;
        cout << endl;

        for (i = 0; i < 5; i++)
            cout << rec_arr.data[i] << endl;
        cout << endl;

        cout << rec_var.data << endl;
        cout << endl;

        for (i = 0; i < rec_pnt.received_elements; i++)
            cout << rec_pnt.data[i] << endl;
        cout << endl;

        for (i = 0; i < rec_vec.data.size(); i++)
            cout << rec_vec.data[i].something << " - " << rec_vec.data[i].somethingelse << endl;
        cout << endl;

        cout << "from: " << rec_vec.address << endl; // maybe always different, just for the sake of simplicity

        sock.close();
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
