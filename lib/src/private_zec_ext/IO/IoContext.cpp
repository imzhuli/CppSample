#include <zec_ext/IO/IoContext.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    bool xIoContext::Init()
    {
        _Native.CreateAs<xNativeIoContext>();
        return true;
    }

    void xIoContext::Clean()
    {
        _Native.DestroyAs<xNativeIoContext>();
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        _Native.As<xNativeIoContext>().run_for(std::chrono::microseconds(TimeoutMS));
    }

}
