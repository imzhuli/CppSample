// #include <zec_ext/IO/TcpConnection.hpp>
// #include <iostream>

// using namespace zec;
// using namespace std;


// bool Connected = false;
// static struct : xTcpConnection::iListener
// {
//     virtual void   OnConnected(xTcpConnection * TcpConnectionPtr)  {
//         cout << "OnConnected" << endl;
//         const char * HW = "hello world!";
//         TcpConnectionPtr->PostData(HW, strlen(HW));
//         Connected = true;
//     }
//     /***
//      * OnReceivedData:
//      * called when there is some data in,
//      * @return consumed bytes
//      * */
//     virtual size_t  OnReceiveData(xTcpConnection * TcpConnectionPtr, const void * DataPtr, size_t DataSize) {
//         cout << "OnReceiveData:" << DataSize << endl;
//         return DataSize;
//     }
//     virtual void    OnPeerClose(xTcpConnection * TcpConnectionPtr)  {
//         cout << "OnPeerClose" << endl;
//     }
//     virtual void    OnError(xTcpConnection * TcpConnectionPtr) {
//         cout << "OnError" << endl;}

// } xTestOutput;


// int main(int, char **)
// {
//     xIoContext IoContext;
//     IoContext.Init();

//     xTcpConnection Connection;
//     Connection.Init(&IoContext, "192.168.123.79", 22, &xTestOutput);

//     while(!Connected) {
//         IoContext.LoopOnce(1000);
//     }
//     Connection.Clean();
//     IoContext.Clean();
//     return 0;
// }

int main(int, char **) {}
