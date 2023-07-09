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
 * @file
 * @brief Base64 encode/decode
 * @author Ryan Martell <rdm4@martellventures.com> (with lots of Michael)
 */

#include <climits>
#include <xel_ext/Util/Base64.hpp>

X_NS
{

    /* ---------------- private code */
    static const unsigned char map2[] =
    {
        0x3e, 0xff, 0xff, 0xff, 0x3f, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01,
        0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
        0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
        0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1a, 0x1b,
        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
        0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
        0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33
    };

    size_t Base64Decode(unsigned char *out, const void * in_, size_t out_size)
    {
        auto in = (const unsigned char *)in_;
        size_t i;
        uint32_t v;
        unsigned char *dst = out;

        v = 0;
        for (i = 0; in[i] && in[i] != '='; i++) {
            unsigned int index= in[i]-43;
            if (index >= xel::Length(map2) || map2[index] == 0xff) {
                return 0;
            }
            v = (v << 6) + map2[index];
            if (i & 3) {
                if (static_cast<size_t>(dst - out) < out_size) {
                    *dst++ = (uint8_t)(v >> (6 - 2 * (i & 3)));
                }
            }
        }

        return dst - out;
    }

    /*****************************************************************************
    * b64_encode: Stolen from VLC's http.c.
    * Simplified by Michael.
    * Fixed edge cases and made it work from data (vs. strings) by Ryan.
    *****************************************************************************/

    char * Base64Encode(char *out, size_t out_size, const void * in_, size_t in_size)
    {
        static const char b64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        auto in = (const unsigned char *)in_;
        char *ret, *dst;
        uint32_t i_bits = 0;
        int32_t  i_shift = 0;
        size_t   bytes_remaining = in_size;

        if (in_size >= UINT_MAX / 4 ||
            out_size < X_BASE64_SIZE(in_size)) {
            return NULL;
            }
        ret = dst = out;
        while (bytes_remaining) {
            i_bits = (i_bits << 8) + *in++;
            bytes_remaining--;
            i_shift += 8;

            do {
                *dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
                i_shift -= 6;
            } while (i_shift > 6 || (bytes_remaining == 0 && i_shift > 0));
        }
        while ((dst - ret) & 3) {
            *dst++ = '=';
        }
        *dst = '\0';

        return ret;
    }

}
