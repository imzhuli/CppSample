#pragma once
#include <zec/Common.hpp>
#include "./Mouse.hpp"

ZEC_NS
{

    struct iGameWindow
    {
        virtual void Create()  = 0;
        virtual void Destroy() = 0;

        void ShowMouse(bool Enable);
        void SetMouseMode(eMouseMode Mode);
        void SetFullScreen(bool FullScreen);
    };



}
