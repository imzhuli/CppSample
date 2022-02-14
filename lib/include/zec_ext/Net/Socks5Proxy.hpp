#pragma once

#include <zec/Common.hpp>
#include <string_view>

ZEC_NS
{

    class iProxyAuth
    {
        virtual bool CheckUserPass(const std::string_view & Username, const std::string_view & Password) = 0;
    };

    class xS5Proxy
    {
    public:
        bool Init(uint16_t port, iProxyAuth * AuthCallback) {
            assert(!_AuthCallback);
            _AuthCallback = AuthCallback;
            return true;
        };
        void Clean() {
            assert(_AuthCallback);
            Reset(_AuthCallback);
        }
        void Run();
        void Stop();

    private:
        iProxyAuth * _AuthCallback = {};
    };

}