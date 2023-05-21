#include <xel_ext/IO/LocalDns.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>

X_NS
{

    static constexpr const uint64_t QuerTimeoutMS = 3'000;

    static inline bool TestAndReduceSize(size_t & Size, size_t RequiredSize)
    {
        if (Size < RequiredSize) {
            return false;
        }
        Size -= RequiredSize;
        return true;
    }

    static void NoNotify(xVariable Variable)
    {
        Pass();
    }

    bool xLocalDnsServer::Init(const xNetAddress & ServerAddress, NotifyCallback * NotifyCallbackPtr, xVariable NotifyVariable)
    {
        assert(ServerAddress);
        if (!IoContext.Init()) {
            return false;
        }
        auto IoContextGuard = MakeCleaner(IoContext);

        auto BindAddress = ServerAddress.IsV4() ? xNetAddress::Make4() : xNetAddress::Make6();
        if (!UdpChannel.Init(&IoContext, BindAddress, this)) {
            return false;
        }
        auto UdpChannelGuard = MakeCleaner(UdpChannel);

        if (!IdPool.Init(MaxQueryCount)) {
            return false;
        }
        assert(IdMarks.empty());
        IdMarks.resize(MaxQueryCount);
        auto IdPoolGuard = MakeCleaner(IdPool);

        DnsServerAddress = ServerAddress;
        if (NotifyCallbackPtr) {
            this->NotifyCallbackPtr = NotifyCallbackPtr;
            this->NotifyVariable = NotifyVariable;
        } else {
            this->NotifyCallbackPtr = &NoNotify;
            this->NotifyVariable = {};
        }

        IdPoolGuard.Dismiss();
        UdpChannelGuard.Dismiss();
        IoContextGuard.Dismiss();

        StopFlag = false;
        ServiceThread = std::thread([this]{ this->Loop(); });
        return true;
    }

    void xLocalDnsServer::Clean()
    {
        StopFlag = true;
        ServiceThread.join();
        Renew(ServiceThread);
        Reset(NotifyCallbackPtr);
        Reset(NotifyVariable);

        Renew(IdMarks);
        IdPool.Clean();
        UdpChannel.Clean();
        IoContext.Clean();
    }

    void xLocalDnsServer::Loop()
    {
        while(!StopFlag) {
            // IO Loop
            IoContext.LoopOnce(100);
            auto NowMS = GetTimestampMS();

            // pick requests
            do {
                xList<xRequest> TempRequestList;
                do {
                    auto Guard = xSpinlockGuard(Spinlock);
                    TempRequestList.GrabListTail(RequestList);
                } while(false);
                for (auto & Request : TempRequestList) {
                    if (DoSendDnsQuery(&Request)) {
                        Request.TimestampMS = NowMS;
                        RequestTimeoutList.GrabTail(Request);
                    } else {
                        InternalRequestResultList.GrabTail(Request);
                    }
                }
            } while(false);

            // Check Timeout
            do {
                auto KillTimepoint = NowMS - QuerTimeoutMS;
                for (auto & Request : RequestTimeoutList) {
                    if (SignedDiff(Request.TimestampMS, KillTimepoint) < 0) {
                        break;
                    }
                    InternalRequestResultList.GrabTail(Request);
                }
            } while(false);

            if (!InternalRequestResultList.IsEmpty()) {
                Spinlock.SynchronizedCall([this]{ExchangeRequestResultList.GrabListTail(InternalRequestResultList);});
                (*NotifyCallbackPtr)(NotifyVariable);
            }

        } // end of while

        do { // clear the queries given up.
            InternalRequestResultList.GrabListTail(RequestTimeoutList);
            auto Guard = xSpinlockGuard(Spinlock);
            ExchangeRequestResultList.GrabListTail(InternalRequestResultList);
        } while(false);

    }

    void xLocalDnsServer::OnError(xUdpChannel * ChannelPtr)
    {
        Fatal("UdpChannelError");
    }

    void xLocalDnsServer::OnData (xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress)
    {
        auto Hex = xel::HexShow(DataPtr, DataSize);
        // X_DEBUG_PRINTF("UdpData: \n%s\n", Hex.c_str());

        auto Reader = xStreamReader(DataPtr);
        if (!TestAndReduceSize(DataSize, 12)) { // invalid response, drop it
            return;
        }
        // typedef struct {
        //     uint16_t     xid;      /* Randomly chosen identifier */
        //     uint16_t     flags;    /* Bit-mask to indicate request/response */
        //     uint16_t     qdcount;  /* Number of questions */
        //     uint16_t     ancount;  /* Number of answers */
        //     uint16_t     nscount;  /* Number of authority records */
        //     uint16_t     arcount;  /* Number of additional records */
        // } dns_header_t;
        auto Index = Reader.R2();
        auto Flags = Reader.R2();
        auto QCount = Reader.R2();
        auto ACount = Reader.R2();
        auto NSCount = Reader.R2();
        auto ARCount = Reader.R2();
        (void)NSCount;
        (void)ARCount;

        if (!(Flags & 0x8000)) {
            // X_DEBUG_PRINTF("xLocalDnsServer::OnData not an dns answer\n");
            return; // not an answer
        }
        if (QCount != 1 || !ACount) {
            // X_DEBUG_PRINTF("xLocalDnsServer::OnData qcount != 1 or acount == 0.\n");
            return;
        }

        /* Check Query */
        Reader.Reset(Reader);
        char HostnameBuffer[256 + 1]; // terminal-null added
        size_t TotalHostnameLength = 0;
        while(true) {
            if (TotalHostnameLength) {
                HostnameBuffer[TotalHostnameLength++] = '.';
            }
            if (!TestAndReduceSize(DataSize, 1)) { // invalid response, drop it
                // X_DEBUG_PRINTF("xLocalDnsServer::OnData Invalid domain segment length. \n");
                return;
            }
            auto SegLength = Reader.R1();
            if (!SegLength) {
                if (TotalHostnameLength) {
                    HostnameBuffer[--TotalHostnameLength] = '\0';
                }
                break;
            }
            if (!TestAndReduceSize(DataSize, SegLength)) { // invalid response, drop it
                // X_DEBUG_PRINTF("xLocalDnsServer::OnData Invalid domain segment length. \n");
                return;
            }
            Reader.R(HostnameBuffer + TotalHostnameLength, SegLength);
            TotalHostnameLength += SegLength;
        }
        auto QueryType = Reader.R2();
        auto QueryClass = Reader.R2();
        if (QueryType != 1 || QueryClass != 1) {
            // X_DEBUG_PRINTF("xLocalDnsServer::OnData unsupported query type=%i class=%i\n", (int)QueryType, (int)QueryClass);
            return;
        }

        /* Extract answers */
        size_t ResolvedCounter = 0;
        xNetAddress Resolved[3] = {};
        while(ACount--)
        {
            if (ResolvedCounter >= Length(Resolved)) {
                break;
            }
            if (!TestAndReduceSize(DataSize, 12)) { // invalid response, drop it
                X_DEBUG_PRINTF("xLocalDnsServer::OnData Invalid record header length. \n");
                return;
            }

            // typedef struct dns_record_base_t {
            //   uint16_t compression;
            //   uint16_t dnstype;
            //   uint16_t dnsclass;
            //   uint32_t ttl;
            //   uint16_t length;
            // } __attribute__((packed)) dns_record_base_t;

            // typedef struct dns_record_a4_t {
            //   struct dns_record_base_t base;
            //   struct in_addr addr;
            // } __attribute__((packed)) dns_record_a4_t;

            // typedef struct dns_record_a6_t {
            //   struct dns_record_base_t base;
            //   struct in6_addr addr;
            // } __attribute__((packed)) dns_record_a6_t;
            auto Compression = Reader.R2();
            auto Type = Reader.R2();
            auto Class = Reader.R2();
            auto Ttl = Reader.R4();
            auto RecordLength = Reader.R2();
            (void)Compression;
            (void)Ttl;
            if (!TestAndReduceSize(DataSize, RecordLength)) { // invalid response, drop it
                // X_DEBUG_PRINTF("xLocalDnsServer::OnData Invalid record length. \n");
                return;
            }
            if (Type == 1 && Class == 1) { /* INET4 && IN */
                if (RecordLength != 4) {
                    X_DEBUG_PRINTF("xLocalDnsServer::OnData Invalid ipv4 address. \n");
                    return;
                }
                auto & Address4 = Resolved[ResolvedCounter++];
                Address4 = xNetAddress::Make4();
                Reader.R(Address4.Ipv4, 4);
                // X_DEBUG_PRINTF("Resolved record, %s\n", Address4.IpToString().c_str());
                continue;
            }
            else {
                // X_DEBUG_PRINTF("Ignore record, type=%i, class=%i\n", (int)Type, (int)Class);
            }
            Reader.Skip(RecordLength);
        }

        DoPushResolvResult(Index, HostnameBuffer, Resolved, ResolvedCounter);
        return;
    }

    bool xLocalDnsServer::DoSendDnsQuery(xRequest * RequestPtr)
    {
        assert(RequestPtr);
        assert(!RequestPtr->Ident);
        auto Ident = IdPool.Acquire(RequestPtr);
        if (!Ident) {
            return false;
        }
        RequestPtr->Ident = Ident;

        auto Index = (uint16_t)Ident;
        assert(!IdMarks[Index]);
        IdMarks[Index] = true;

        ubyte Buffer[1024];
        auto Writer = xStreamWriter(Buffer);

        // typedef struct {
        //     uint16_t     xid;      /* Randomly chosen identifier */
        //     uint16_t     flags;    /* Bit-mask to indicate request/response */
        //     uint16_t     qdcount;  /* Number of questions */
        //     uint16_t     ancount;  /* Number of answers */
        //     uint16_t     nscount;  /* Number of authority records */
        //     uint16_t     arcount;  /* Number of additional records */
        // } dns_header_t;
        Writer.W2((uint16_t)Index);
        Writer.W2(0x0100); /* Q=0, Recursion Desired=0 */
        Writer.W2(1); /* Questions */
        Writer.W2(0);
        Writer.W2(0);
        Writer.W2(0);

        // typedef struct {
        //     char *       name;        /* domain name in memory */
        //     uint16_t     dnstype;  /* The QTYPE (1 = A) */
        //     uint16_t     dnsclass; /* The QCLASS (1 = IN) */
        // } dns_question_t;

        /* Write name*/
        size_t Counter = 0;
        const char * Hostname = RequestPtr->Hostname.data();
        while(auto C = *Hostname) {
            if (C == '.') {
                Writer.W1(Counter);
                Writer.W(Hostname - Counter, Counter);
                Counter = 0;
                ++Hostname;
                continue;
            }
            ++Counter;
            ++Hostname;
        }
        Writer.W1(Counter);
        Writer.W(Hostname - Counter, Counter);
        Writer.W(0);

        Writer.W2(1); /* type A */
        Writer.W2(1); /* IN */

        // X_DEBUG_PRINTF("DnsQuery: \n%s\n", HexShow(Writer.Origin(), Writer.Offset()).c_str());
        UdpChannel.PostData(Writer.Origin(), Writer.Offset(), DnsServerAddress);
        return true;
    }

    void xLocalDnsServer::DoPushResolvResult(uint16_t Index, const char * HostnameBuffer, const xNetAddress * ResolvedList, size_t ResolvedCounter)
    {
        if (!IdMarks[Index]) {
            X_DEBUG_PRINTF("xLocalDnsServer::DoPushResolvResult Invalid ident. \n");
            return;
        }
        auto RequestPtr = IdPool[Index];
        if (0 != strcmp(HostnameBuffer, RequestPtr->Hostname.c_str())) {
            X_DEBUG_PRINTF("xLocalDnsServer::DoPushResolvResult Query hostname don't match. \n");
            return;
        }
        if (TestAndReduceSize(ResolvedCounter, 1)) {
            RequestPtr->Address1 = ResolvedList[1];
        }
        if (TestAndReduceSize(ResolvedCounter, 1)) {
            RequestPtr->Address2 = ResolvedList[2];
        }
        ReleaseQuery(Index);
        return;
    }

    void xLocalDnsServer::ReleaseQuery(uint16_t Index)
    {
        if (!IdMarks[Index]) {
            return;
        }
        auto RequestPtr = IdPool[Index];
        assert(IdPool.Check(RequestPtr->Ident));
        assert(Index == static_cast<uint16_t>(RequestPtr->Ident));

        // remove it from list
        IdMarks[Index] = false;
        IdPool.Release(Steal(RequestPtr->Ident));
        InternalRequestResultList.GrabTail(*RequestPtr);
    }

    void xLocalDnsServer::PostQuery(xRequest * RequestPtr)
    {
        do {
            assert(!xListNode::IsLinked(*RequestPtr));
            auto Guard = xSpinlockGuard(Spinlock);
            RequestList.GrabTail(*RequestPtr);
        } while(false);
        IoContext.GetUserEventTrigger()->Trigger();
    }

    void xLocalDnsServer::Pick(xList<xRequest> & Receiver)
    {
        do {
            auto Guard = xSpinlockGuard(Spinlock);
            Receiver.GrabListTail(ExchangeRequestResultList);
        } while(false);
    }

}
