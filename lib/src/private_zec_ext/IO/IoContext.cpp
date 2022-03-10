#include <zec_ext/IO/IoContext.hpp>
#include "./_Local.hpp"

#include <iostream>
using namespace std;

ZEC_NS
{

    using xIoContextWorkGuard = asio::executor_work_guard<asio::io_context::executor_type>;
    static constexpr const size_t WorkGuardSize = sizeof(xIoContextWorkGuard);

    bool xIoContext::Init()
    {
        static_assert(_WorkGuard.Size >= WorkGuardSize);
        _WorkGuard.CreateValueAs<xIoContextWorkGuard>(
            asio::make_work_guard(_Native.CreateValueAs<xNativeIoContext>(1))
            );
        return true;
    }

    void xIoContext::Clean()
    {
        _WorkGuard.DestroyAs<xIoContextWorkGuard>();
        _Native.DestroyAs<xNativeIoContext>();
    }

    void xIoContext::LoopOnce(int TimeoutMS)
    {
        auto & Native = _Native.As<xNativeIoContext>();
        Native.run_for(std::chrono::microseconds(TimeoutMS));
        for (auto & ExState : _ResumeList) {
            ExState.Detach();
            ExState.OnResume();
        }
    }

}
