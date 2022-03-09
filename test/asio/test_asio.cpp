#include <zec_ext/IO/IoContext.hpp>
#include <zec_ext/IO/Resolver.hpp>
#include <zec_ext/IO/TcpConnection.hpp>
#include <zec/Util/Chrono.hpp>
#include <zec/String.hpp>
#include <iostream>
#include <fstream>
#include <memory>

using namespace zec;
using namespace std;

xIoContext IoContext;
static std::string TestHostname = "www.baidu.com";

static std::string Request =
"GET /img/PC_880906d2a4ad95f5fafb2e540c5cdad7.png HTTP/1.1\r\n"
"Host: www.baidu.com\r\n"
"Connection: keep-alive\r\n"
"Pragma: no-cache\r\n"
"Cache-Control: no-cache\r\n"
"sec-ch-ua: \" Not A;Brand\";v=\"99\", \"Chromium\";v=\"98\", \"Microsoft Edge\";v=\"98\"\r\n"
"sec-ch-ua-mobile: ?0\r\n"
"sec-ch-ua-platform: \"Windows\"\r\n"
"Upgrade-Insecure-Requests: 1\r\n"
"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/98.0.4758.80 Safari/537.36 Edg/98.0.1108.50\r\n"
// "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
"Accept: image/webp,image/apng,image/svg+xml,image/*,*/*;q=0.8;\r\n"
"Sec-Fetch-Site: none\r\n"
"Sec-Fetch-Mode: navigate\r\n"
"Sec-Fetch-User: ?1\r\n"
"Sec-Fetch-Dest: document\r\n"
"Accept-Encoding: gzip, deflate, br\r\n"
"Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6,zh-TW;q=0.5,ja;q=0.4\r\n"
"Cookie: BIDUPSID=BB40908B823E3813A592B6A7F56E70FD; PSTM=1638683320; BAIDUID=BB40908B823E381302C388293D6CFC8A:FG=1; __yjs_duid=1_0869737fc666f99cd0d45cf56ef256be1638683386620; BD_UPN=12314753; BAIDUID_BFESS=BB40908B823E381302C388293D6CFC8A:FG=1; BD_HOME=1; BDRCVFR[vp1ya5kbS93]=mk3SLVN4HKm; delPer=0; BD_CK_SAM=1; RT=\"z=1&dm=baidu.com&si=8c6n67yhifh&ss=kzs2vxu7&sl=3&tt=140&bcn=https%3A%2F%2Ffclog.baidu.com%2Flog%2Fweirwood%3Ftype%3Dperf&ld=4f0&cl=3pr&ul=4hz&hd=4i5\"; COOKIE_SESSION=34_0_4_4_10_1_1_0_4_1_0_0_33_0_1_0_1645169026_0_1645169027%7C9%23386023_7_1640579706%7C4; PSINO=6; BDORZ=FFFB88E999055A3F8A630C64834BD6D0; H_PS_645EC=f231MhpwMUStRAd7GSc6op6jpwIpqAKccavLx1Mb7C%2BSltFN7oDWM5xmgN%2FeTvJraD4R7P%2BFPrOs; H_PS_PSSID=35106_31253_35762_35489_34584_35490_35842_35542_35317_26350_35868; BA_HECTOR=20al2lagakal24aksi1h11kn30r\r\n"
"\r\n";

class xTcpTest
: public xTcpResolver::iListener
, public xTcpConnection::iListener
{
private:
    void OnResolve(const std::string & Hostname, const xNetAddress & Address, const xVariable & RequestContext, bool DirectCall) override
    {
        cout << "Resolve result: " << Hostname << ", RequestId=" << RequestContext.I << ", Result=" << Address.ToString() << ", Cached=" << YN(DirectCall) << endl;
        if (Hostname != TestHostname) {
            return;
        }
        auto VisitAddress = Address;
        VisitAddress.Port = 80;
        if (!TcpConnection.Init(IoContextPtr, VisitAddress, this)) {
            cerr << "Failed to init connection" << endl;
            return;
        }
        TcpConnection.PostData(Request.data(), Request.length());
    }

    void OnConnected(xTcpConnection * TcpConnectionPtr)
    {
        assert(&TcpConnection == TcpConnectionPtr);
        cout << "Connected" << endl;
        TcpConnectionPtr->SuspendReading();
        TcpConnectionPtr->ResumeReading();
    }

    size_t Counter = 0;
    size_t OnData(xTcpConnection * TcpConnectionPtr, const void * DataPtr, size_t DataSize) override
    {
        assert(&TcpConnection == TcpConnectionPtr);
        Data.append((const char *)DataPtr, DataSize);
        cout << "Data: " << endl;
        cout << HexShow(DataPtr, DataSize) << endl;
        TcpConnectionPtr->SuspendReading();
        TcpConnectionPtr->ResumeReading();
        return DataSize;
    }

    void OnPeerClose(xTcpConnection * TcpConnectionPtr) override
    {
        cout << "PeerClose" << endl;
    }

    void OnError(xTcpConnection * TcpConnectionPtr) override
    {
        cerr << "Error" << endl;
    }

    xTcpConnection    TcpConnection;
    xIoContext *      IoContextPtr = nullptr;
    std::string       Data;

public:
    bool Init(xIoContext * IoContextPtr) {
        this->IoContextPtr = IoContextPtr;
        return true;
    }

    void Clean() {
        if (!Data.empty()) {
            auto Index = Data.find("\r\n\r\n");
            cout << "HeaderSize:" << Index << endl;
            Data = Data.substr(Index + 4);
            std::ofstream output ("/tmp/some.png", ios::binary);
            output.write(Data.data(), Data.length());
            output.close();
        }
        TcpConnection.Clean();
    }
};

void test0 ()
{
    xTcpResolver           Resolver;
    xTcpTest               Tester;

    if (!IoContext.Init()) {
        throw "Failed to init io context";
    }

    if (!Tester.Init(&IoContext)) {
        throw "Failed to init test object";
    }

    if (!Resolver.Init(&IoContext, &Tester)) {
        throw "Failed to init resolver";
    }

    Resolver.Request(TestHostname, { .I = 1 });
    xTimer Timer;
    while(!Timer.TestAndTag(4s)) {
        Resolver.ClearTimeoutRequest();
        IoContext.LoopOnce(100);
    }
    Resolver.ClearTimeoutCacheNode();

    Resolver.Clean();
    cout << "Resolver cleaned" << endl;

    Tester.Clean();
    cout << "Tester cleaned" << endl;

    IoContext.Clean();
    cout << "IoContext cleaned" << endl;
}

int main(int, char **)
{
    try {
        test0();
    }
    catch (const char * Reason) {
        cout << Reason << endl;
        return -1;
    }
    // catch (...)
    // {
    //     return -1;
    // }
    return 0;
}
