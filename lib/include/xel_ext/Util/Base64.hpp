#pragma once
#include <xel/Common.hpp>

X_NS
{

    /**
     * Calculate the output size needed to base64-encode x bytes.
     */
    #define X_BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

    /*
    * Copyright (c) 2006 Ryan Martell. (rdm4@martellventures.com)
    *
    * This file is part of FFmpeg.
    *
    * FFmpeg is free software; you can redistribute it and/or
    * modify it under the terms of the GNU Lesser General Public
    * License as published by the Free Software Foundation; either
    * version 2.1 of the License, or (at your option) any later version.
    *
    * FFmpeg is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    * Lesser General Public License for more details.
    *
    * You should have received a copy of the GNU Lesser General Public
    * License along with FFmpeg; if not, write to the Free Software
    * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
    */

    /**
     * Decode a base64-encoded string.
     *
     * @param out      buffer for decoded data
     * @param in       null-terminated input string
     * @param out_size size in bytes of the out buffer, must be at
     *                 least 3/4 of the length of in
     * @return         number of bytes written, or a negative value in case of
     *                 invalid input
     */
    X_API size_t Base64Decode(unsigned char * out, const void *in, size_t out_size);

    /**
     * Encode data to base64 and null-terminate.
     *
     * @param out      buffer for encoded data
     * @param out_size size in bytes of the output buffer, must be at
     *                 least X_BASE64_SIZE(in_size)
     * @param in_size  size in bytes of the 'in' buffer
     * @return         'out' or NULL in case of error
     */
    X_API char * Base64Encode(char *out, size_t out_size, const void * in, size_t in_size);

}
