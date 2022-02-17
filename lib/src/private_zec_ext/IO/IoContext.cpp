#include <zec_ext/IO/IoContext.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    bool xIoContext::Init()
    {
        auto & IoContextHolder = GetHolderRef(this);
        IoContextHolder.Create();
        return IoContextHolder.IsValid();
    }

    void xIoContext::Clean()
    {
        auto & IoContextHolder = GetHolderRef(this);
        IoContextHolder.Destroy();
    }

}
