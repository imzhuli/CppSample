#include <zec_ext/IO/IoContext.hpp>
#include "./_Local.hpp"

ZEC_NS
{

    bool xIoContext::Init()
    {
        _Native.CreateValueAs<xNativeIoContext>(1);
        return true;
    }

    void xIoContext::Clean()
    {
        _Native.DestroyAs<xNativeIoContext>();
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        _Native.As<xNativeIoContext>().run_for(std::chrono::microseconds(TimeoutMS));
        for (auto & ExState : _ResumeList) {
            ExState.Detach();
            ExState.OnResume();
        }
    }

}
