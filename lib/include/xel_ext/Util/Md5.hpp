#pragma once

#include <xel/Common.hpp>

X_NS
{
    #define X_MD5_DIGEST_SIZE 16
    X_API void Md5(xel::ubyte resblock[X_MD5_DIGEST_SIZE], const void *buffer, size_t len);
}
