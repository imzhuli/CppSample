#include "../base/_.hpp"
#include <xel_ext/Util/Base64.hpp>
using namespace std;
using namespace xel;

static xIoContext  IoContext;
static xNetAddress HttpProxyAddress;

enum eS5State {
    S5_INIT,
    S5_AUTH,
    S5_CONNECTING,
    S5_WAITING_FOR_REMOTE,
    S5_CONNECTED,
    S5_CLOSED,
};

enum eHttpState {
    HTTP_NOT_PREPARED,
    HTTP_CONNECTING,
    HTTP_READY,
};
struct xProxyConnection : xTcpConnection {
    void * Outer = nullptr;
};

struct xS5Client : xTcpConnectionEx
{
    eS5State          S5State = S5_INIT;
    eHttpState        HttpState = HTTP_NOT_PREPARED;

    string            S5Name;
    string            S5Password;
    string            HttpPassword;
    xProxyConnection  ProxyConnection;
};
static xTcpConnectionExList S5ConnectionList;
static xTcpConnectionExList S5ConnectionKillList;
static xTcpConnectionManager<xS5Client> S5ConnectionManager;

void Kill(xS5Client* S5ClientPtr) { S5ConnectionKillList.GrabTail(*S5ClientPtr); }

struct xHttpConnectionListener
: xTcpConnection::iListener
{
    size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) {
        auto ConnectionPtr = static_cast<xProxyConnection*>(TcpConnectionPtr);
        auto S5Ptr = static_cast<xS5Client*>(ConnectionPtr->Outer);
        if (S5Ptr->HttpState == HTTP_CONNECTING) {
            cout << "HttpConnectResponse: " << HexShow(DataPtr, DataSize) << endl;
            auto View = string_view((const char*)DataPtr, DataSize);
            auto EndIndex = View.find("\r\n\r\n");
            if (EndIndex == View.npos) {
                return 0;
            }
            if (EndIndex != View.size() - 4) {
                cerr << "Invalid Http Response" << endl;
                Kill(S5Ptr);
                return 0;
            }
            // Check 200/Connection established
            auto SuccessIndex = View.find("200 Connection established\r\n\r\n");
            if (SuccessIndex == View.npos) {
                cerr << "Failed to build Http Proxy Connection" << endl;
                Kill(S5Ptr);
                return 0;
            }
            if (S5Ptr->S5State != S5_WAITING_FOR_REMOTE) {
                cerr << "Invalid S5 state" << endl;
                Kill(S5Ptr);
                return 0;
            }

            static constexpr const ubyte Reply[] = { '\x05', '\x00', '\x00', // ok
                '\x01', // ipv4
                '\x00', '\x00', '\x00', '\x00', // ip: 0.0.0.0
                '\x00', '\x00', // port 0:
            };
            S5Ptr->PostData(Reply, sizeof(Reply));
            S5Ptr->S5State = S5_CONNECTED;
            S5Ptr->HttpState = HTTP_READY;
            return DataSize;
        }
        if (S5Ptr->HttpState == HTTP_READY) {
            S5Ptr->PostData(DataPtr, DataSize);
            return DataSize;
        }

        cerr << "Invalid http status" << endl;
        Kill(S5Ptr);
        return DataSize;
    }
    void OnPeerClose(xTcpConnection * TcpConnectionPtr) {
        auto ConnectionPtr = static_cast<xProxyConnection*>(TcpConnectionPtr);
        auto S5Ptr = static_cast<xS5Client*>(ConnectionPtr->Outer);
        Kill(S5Ptr);
    }
};
[[maybe_unused]]
static xHttpConnectionListener HttpProxyListener;

string MakeHttpConnectHeader(const string & Target, const string & Username, const string & Password)
{
    auto AuthSource = Username + ':' + Password;
    auto AuthBase64 = vector<char>(X_BASE64_SIZE(AuthSource.size()) + 64);
    auto OutPtr = Base64Encode(AuthBase64.data(), AuthBase64.size(), AuthSource.data(), AuthSource.size());
    if (!OutPtr) {
        return {};
    }

    stringstream OSS;
    OSS << "CONNECT " << Target << " HTTP/1.1" << "\r\n";
    OSS << "Proxy-Connection: Keep-Alive" << "\r\n";
    OSS << "Proxy-Authorization:Basic " << OutPtr << "\r\n";
    OSS << "Content-Length: 0" << "\r\n";
    OSS << "\r\n";
    return OSS.str();
}

struct xS5ClientListener
: xTcpServer::iListener
, xTcpConnection::iListener
{
    void OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle)
    {
        auto S5ClientPtr = S5ConnectionManager.CreateConnection();
        if (!S5ClientPtr) {
            cerr << "Failed to alloc connectin" << endl;
            XelCloseSocket(NativeHandle);
            return;
        }
        if (!S5ClientPtr->Init(&IoContext, std::move(NativeHandle), this)) {
            cerr << "Failed to init connection" << endl;
            S5ConnectionManager.DestroyConnection(S5ClientPtr);
            return;
        }
        S5ClientPtr->ProxyConnection.Outer = S5ClientPtr;
        S5ConnectionList.GrabTail(*S5ClientPtr);
        cout << "CreateConnection cid=" << S5ClientPtr->ConnectionId << endl;
    }

    size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize)
    {
        auto S5Ptr = static_cast<xS5Client*>(TcpConnectionPtr);
        auto Reader = xStreamReader(DataPtr);
        if (S5Ptr->S5State == S5_INIT) {
            if (DataSize < 3) {
                return 0;
            }
            if (Reader.R1() != 0x05) {
                cerr << "Invalid s5 challenge: " << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }
            auto NM = Reader.R1(); // number of methods
            if (!NM) {
                cerr << "Invalid number of methods: " << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }
            auto ChallengeSize = size_t(2 + NM); // s5 challenge 1st challenge head
            if (DataSize < ChallengeSize) {
                return 0; // wait for more bytes;
            }
            bool UserPassSupport = false;
            for (size_t i = 0; i < NM; ++i) {
                uint8_t Method = Reader.R1();
                if (Method == 0x02) {
                    UserPassSupport = true;
                    break;
                }
            }
            if (!UserPassSupport) {
                cerr << "S5 user-password suppported disabled: " << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }

            ubyte Socks5Auth[2] = { 0x05, 0x02 };
            S5Ptr->PostData(Socks5Auth, sizeof(Socks5Auth));
            S5Ptr->S5State = S5_AUTH;
            return ChallengeSize;
        }

        if (S5Ptr->S5State == S5_AUTH) {
            if (DataSize < 3) {
                return 0;
            }
            auto Ver = Reader.R1();
            if (Ver != 0x01) {
                cerr << "invalid user-pass protocol: " << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }
            auto NameLen = (size_t)Reader.R1();
            if (DataSize < 3 + NameLen) { // wait for more
                return 0;
            }
            auto NameView = string_view{ (const char *)Reader.Skip(NameLen), NameLen };

            auto PassLen = (size_t)Reader.R1();
            if (DataSize < 3 + NameLen + PassLen) {// wait for more
                return 0;
            }
            auto PassView = string_view{ (const char *)Reader.Skip(PassLen), PassLen};
            cout << "S5Auth cid=" << S5Ptr->ConnectionId << ", user=" << NameView << ", pass=" << PassView << endl;

            S5Ptr->PostData("\x01\x00", 2); // always send ok

            S5Ptr->S5Name = string(NameView);
            S5Ptr->S5Password = string(PassView);
            S5Ptr->S5State = S5_CONNECTING;
            return Reader.Offset();
        }

        if (S5Ptr->S5State == S5_CONNECTING) {
            if (DataSize < 10) { // wait for more
                return 0;
            }
            if (DataSize >= 6 + 256) {
                cerr << "Very big connection request, which is obviously wrong: cid=" << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }
            uint8_t Version   = Reader.R1();
            uint8_t Operation = Reader.R1();
            uint8_t Reserved  = Reader.R1();
            uint8_t AddrType  = Reader.R1();
            if (Version != 0x05 || Reserved != 0x00) {
                cerr << "Non Socks5 connection request: cid=" << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }
            xNetAddress Address;
            char        DomainName[256];
            size_t      DomainNameLength = 0;
            if (AddrType == 0x01) { // ipv4
                Address.Type = xNetAddress::eIpv4;
                Reader.R(Address.Ipv4, 4);
                Address.Port = Reader.R2();
            }
            else if (AddrType == 0x03) {
                DomainNameLength = Reader.R();
                if (DataSize < 4 + 1 + DomainNameLength + 2) { // wait for more
                    return 0;
                }
                Reader.R(DomainName, DomainNameLength);
                DomainName[DomainNameLength] = '\0';
                Address.Port = Reader.R2();
            } else if (AddrType == 0x04) { // ipv6
                if (DataSize < 4 + 16 + 2) { // wait for more
                    return 0;
                }
                Address.Type = xNetAddress::eIpv6;
                Reader.R(Address.Ipv6, 16);
                Address.Port = Reader.R2();
            } else {
                cerr << "Invalid connection request, address type invalid: cid=" << S5Ptr->ConnectionId << endl;
                Kill(S5Ptr);
                return 0;
            }
            size_t AddressLength = Reader.Offset() - 3;
            if (Operation != 0x01) {
                cerr << "Invalid connection request, operation type invalid: cid=" << S5Ptr->ConnectionId << endl;
                ubyte Buffer[512];
                xStreamWriter W(Buffer);
                W.W(0x05);
                W.W(0x01);
                W.W(0x00);
                W.W((ubyte*)DataPtr + 3, AddressLength);
                S5Ptr->PostData(Buffer, W.Offset());
                Kill(S5Ptr);
                return 0;
            }

            auto Target = string();
            if (AddrType == 0x03) { // connection based on domain
                Target = string(DomainName, DomainNameLength) + ':' + to_string(Address.Port);
            } else {
                Target = Address.ToString();
            }
            cout << "S5 connection request, cid=" << S5Ptr->ConnectionId << ", target=" << Target << endl;

            auto Header = MakeHttpConnectHeader(Target, S5Ptr->S5Name, S5Ptr->S5Password);
            if (Header.empty()) {
                cerr << "Failed to make http connect header" << endl;
                Kill(S5Ptr);
                return 0;
            }

            if (!S5Ptr->ProxyConnection.Init(&IoContext, HttpProxyAddress, &HttpProxyListener)) {
                cerr << "Failed to make proxy connection" << endl;
                Kill(S5Ptr);
                return 0;
            }

            S5Ptr->ProxyConnection.PostData(Header.data(), Header.size());
            S5Ptr->S5State = S5_WAITING_FOR_REMOTE;
            S5Ptr->HttpState = HTTP_CONNECTING;
            return Reader.Offset();
        }

        if (S5Ptr->S5State == S5_CONNECTED) {
            S5Ptr->ProxyConnection.PostData(DataPtr, DataSize);
            return DataSize;
        }

        // not implemented
        cerr << "Not implemented" << endl;
        Kill(S5Ptr);
        return 0;
    }

    void OnPeerClose(xTcpConnection * TcpConnectionPtr)  {
        auto S5Ptr = static_cast<xS5Client*>(TcpConnectionPtr);
        Kill(S5Ptr);
    }
};

static xS5ClientListener S5Listener;
static xTcpServer TcpServer;

void DestroyConnections() {
    for (auto & Node : S5ConnectionKillList) {
        auto S5ClientPtr = static_cast<xS5Client*>(&Node);
        if (S5ClientPtr->HttpState != HTTP_NOT_PREPARED) {
            S5ClientPtr->ProxyConnection.Clean();
        }
        S5ClientPtr->Clean();
        cout << "DestroyConnection: cid=" << S5ClientPtr->ConnectionId << endl;
        S5ConnectionManager.DestroyConnection(S5ClientPtr);
    }
}

int main(int argc, char * argv[])
{
    auto Cmd = xCommandLine(argc, argv, {
        { 'h', "http_proxy",     "http_proxy", true },
        { 'a', "bind_address",   "bind_address",  true },
    });
    auto OptProxy = Cmd["http_proxy"];
    if (!OptProxy()) {
        cerr << "missing http proxy address" << endl;
        return -1;
    };
    HttpProxyAddress = xNetAddress::Parse(*OptProxy);
    if (!HttpProxyAddress || !HttpProxyAddress.Port) {
        cerr << "invalid http proxy address" << endl;
        return -1;
    }

    auto BindAddress = xNetAddress::Parse("0.0.0.0:22222");
    auto OptBindAddress = Cmd["bind_address"];
    if (OptBindAddress()) {
        BindAddress = xNetAddress::Parse(*OptBindAddress);
    }

    auto G_IoContext = xResourceGuard(IoContext);
    auto G_ConnectionManager = xResourceGuard(S5ConnectionManager, 102400);
    auto G_TcpServer = xResourceGuard(TcpServer, &IoContext, BindAddress, &S5Listener, true);

    while(true) {
        IoContext.LoopOnce();
        DestroyConnections();
    }

    return 0;
}
