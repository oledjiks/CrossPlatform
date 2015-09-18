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

        Socket::Address to("127.0.0.1", 10000);

        sock.send<string>(to, "this is the string"); // ("127.0.0.1", 10000, "this is a string");
                                                     // as well as the others

        int iarr[5] = { 0, 1, 2, 3, 4 };
        sock.send<int>(to, iarr, 5);

        sock.send<float>(to, 5.432f);

        double darr[5] = { 0.1, 1.2, 2.3, 3.4, 4.5 };
        sock.send<double>(to, darr, 5);

        prova stct;
        stct.something = 1;
        stct.somethingelse = 1.1f;
        sock.send<prova>(to, stct);

        sock.close();
    }
    catch (Socket::SocketException &e)
    {
        cout << e << endl;
    }

    return 0;
}
