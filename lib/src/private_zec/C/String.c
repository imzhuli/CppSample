#include <zec/C/String.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#define XS_INNER_BUFFER_SIZE        ((size_t)(64))
#define XS_LENGTH_EXTEND_SIZE       ((size_t)256)
#define XS_LENGTH_MAX_EXTEND_SIZE   ((size_t)1024*1024)

typedef struct {
    char * DataPtr;
    size_t DataLength;
    size_t BufferSize;
    char   InnerBuffer[XS_INNER_BUFFER_SIZE];
} __XelString;

XelString XS_New(void)
{
    __XelString * RealPtr = (__XelString*)malloc(sizeof(__XelString));
    RealPtr->DataPtr = RealPtr->InnerBuffer;
    RealPtr->DataPtr[0] = '\0';
    RealPtr->DataLength = 0;
    RealPtr->BufferSize = XS_INNER_BUFFER_SIZE;
    return (XelString)RealPtr;
}

XelString XS_NewData(const char * SourcePtr, size_t Length)
{
    XelString Ret = XS_New();
    XS_SetData(Ret, SourcePtr, Length);
    return Ret;
}

XelString XS_NewString(const char * StringPtr)
{
    XelString Ret = XS_New();
    XS_SetString(Ret, StringPtr);
    return Ret;
}

XelString XS_Duplicate(XelString Str)
{
    XelString Ret = XS_New();
    __XelString* RealPtr = (__XelString *)Str;
    XS_SetData(Ret, RealPtr->DataPtr, RealPtr->DataLength);
    return Ret;
}

void XS_Clear(XelString Str)
{
    __XelString* RealPtr = (__XelString *)Str;
    RealPtr->DataPtr[RealPtr->DataLength = 0] = '\0';
}

void XS_Free(XelString Str)
{
    __XelString* RealPtr = (__XelString *)Str;
    if (RealPtr->DataPtr != RealPtr->InnerBuffer) {
        free(RealPtr->DataPtr);
    }
    free(RealPtr);
}

const char * XS_GetData(XelString Str)
{
    __XelString* RealPtr = (__XelString *)Str;
    return  RealPtr->DataPtr;
}

size_t XS_GetLength(XelString Str)
{
    __XelString* RealPtr = (__XelString *)Str;
    return  RealPtr->DataLength;
}

void XS_SetData(XelString Str, const char * SourcePtr, size_t Length)
{
    __XelString* RealPtr = (__XelString *)Str;
    if (Length < RealPtr->BufferSize) {
        memcpy(RealPtr->DataPtr, SourcePtr, Length);
        RealPtr->DataPtr[RealPtr->DataLength = Length] = '\0';
        return;
    }
    if (RealPtr->DataPtr != RealPtr->InnerBuffer) {
        free(RealPtr->DataPtr);
    }
    size_t WantedSize = Length + XS_LENGTH_EXTEND_SIZE;
    RealPtr->DataPtr = (char*)malloc(WantedSize);
    RealPtr->BufferSize = WantedSize;
    memcpy(RealPtr->DataPtr, SourcePtr, Length);
    RealPtr->DataPtr[RealPtr->DataLength = Length] = '\0';
    return;
}

void XS_SetString(XelString Str, const char * StringPtr)
{
    XS_SetData(Str, StringPtr, strlen(StringPtr));
}

void XS_Append(XelString Str, XelString TailStr)
{
    __XelString* RealPtr = (__XelString *)TailStr;
    XS_AppendData(Str, RealPtr->DataPtr, RealPtr->DataLength);
}

void XS_AppendData(XelString Str, const char * SourcePtr, size_t Length)
{
    __XelString* RealPtr = (__XelString *)Str;
    size_t TotalLength = RealPtr->DataLength + Length;
    if (TotalLength < RealPtr->BufferSize) {
        memcpy(RealPtr->DataPtr + RealPtr->DataLength, SourcePtr, Length);
        RealPtr->DataPtr[RealPtr->DataLength = TotalLength] = '\0';
        return;
    }
    char * OldDataPtr = RealPtr->DataPtr;
    size_t OverallExtendSize = RealPtr->DataLength + XS_LENGTH_EXTEND_SIZE;
    if (OverallExtendSize > XS_LENGTH_MAX_EXTEND_SIZE){
        OverallExtendSize = XS_LENGTH_MAX_EXTEND_SIZE;
    }
    size_t WantedSize = TotalLength + OverallExtendSize;
    RealPtr->DataPtr = (char*)malloc(WantedSize);
    RealPtr->BufferSize = WantedSize;
    memcpy(RealPtr->DataPtr, OldDataPtr, RealPtr->DataLength);
    memcpy(RealPtr->DataPtr + RealPtr->DataLength, SourcePtr, Length);
    RealPtr->DataPtr[RealPtr->DataLength = TotalLength] = '\0';
    if (OldDataPtr != RealPtr->InnerBuffer) {
        free(OldDataPtr);
    }
}

void XS_AppendString(XelString Str, const char * StringPtr)
{
    XS_AppendData(Str, StringPtr, strlen(StringPtr));
}

XelString XS_SubString(XelString Str, size_t Index, size_t Length)
{
    __XelString* RealPtr = (__XelString *)Str;
    if (Index >= RealPtr->DataLength) {
        return XS_New();
    }
    if (Index + Length > RealPtr->DataLength) {
        Length = RealPtr->DataLength - Index;
    }
    return XS_NewData(RealPtr->DataPtr + RealPtr->DataLength, Length);
}

XelString XS_Concat(XelString Str1, XelString Str2)
{
    XelString Ret = XS_Duplicate(Str1);
    XS_Append(Ret, Str2);
    return Ret;
}

XelString XS_HexShow(const void * DataPtr, size_t Length, bool NeedHeader)
{
    if (!Length) {
        return XS_NewString("<empty>");
    }
    const char * hexfmt="%02x";
    const char * buffer = (const char*)DataPtr;
    XelString h = XS_New();
    XelString p = XS_New();
    char lineno[32];
    char ch [32];
    char bh [128];
    char bp [128];
    if (NeedHeader) {
    //	h.append("00000000  0001 0203 0405 0607 0809 0a0b 0c0d 0e0f : ................")
        XS_AppendString(h, "+-Line-+  +-----------------Hex-----------------+   +-----Char-----+\n");
    }
    XS_AppendString(h, "00000000  ");

    size_t oi = 0;
    for (; oi < Length ; ++oi) {
        char c = buffer[oi];
        char cs = (isprint(c) && c != ' ') ? c : '.' ;
        sprintf(ch, hexfmt, (int)(unsigned char)c);
        sprintf(bh, ((oi % 2) ? "%s ": "%s"), ch);
        sprintf(bp, "%c", cs);
        if (oi) {
            if(oi%16 == 0) {
                XS_AppendString(h, "  ");
                XS_Append(h, p);
                XS_AppendString(h, "\n");
                sprintf(lineno, "%08" PRIx32 "  ", (uint32_t)((oi / 16)));
                XS_AppendData(h, lineno, 10);
                XS_Clear(p);
            }
        }
        XS_AppendString(h, bh);
        XS_AppendString(p, bp);
    }
    size_t f = 50 - XS_GetLength(h) % 69;
    XS_AppendData(h,
            "                "
            "                "
            "                "
            "                "
            , f);
    XS_AppendString(h, "  ");
    XS_Append(h, p);
    XS_Free(p);
    return h;
}

void XS_PrintHexShow(FILE * stream, const void * DataPtr, size_t Length, bool NeedHeader)
{
    XelString S = XS_HexShow(DataPtr, Length, NeedHeader);
    if (!S) {
        return;
    }
    fprintf(stream, "%s\n", XS_GetData(S));
    fflush(stream);
    XS_Free(S);
}

static const char CTBL[256] =
{
    0,0,0,0,0,0,0,0          ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,1,2,3,4,5,6,7         ,8,9,0,0,0,0,0,0
    ,0,10,11,12,13,14,15,0   ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,10,11,12,13,14,15,0   ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
    ,0,0,0,0,0,0,0,0         ,0,0,0,0,0,0,0,0
};

static const char HTBL_UPPER[256][2] =
{
    {'0','0'}, {'0','1'}, {'0','2'}, {'0','3'}, {'0','4'}, {'0','5'}, {'0','6'}, {'0','7'}, {'0','8'}, {'0','9'}, {'0','A'}, {'0','B'}, {'0','C'}, {'0','D'}, {'0','E'}, {'0','F'},
    {'1','0'}, {'1','1'}, {'1','2'}, {'1','3'}, {'1','4'}, {'1','5'}, {'1','6'}, {'1','7'}, {'1','8'}, {'1','9'}, {'1','A'}, {'1','B'}, {'1','C'}, {'1','D'}, {'1','E'}, {'1','F'},
    {'2','0'}, {'2','1'}, {'2','2'}, {'2','3'}, {'2','4'}, {'2','5'}, {'2','6'}, {'2','7'}, {'2','8'}, {'2','9'}, {'2','A'}, {'2','B'}, {'2','C'}, {'2','D'}, {'2','E'}, {'2','F'},
    {'3','0'}, {'3','1'}, {'3','2'}, {'3','3'}, {'3','4'}, {'3','5'}, {'3','6'}, {'3','7'}, {'3','8'}, {'3','9'}, {'3','A'}, {'3','B'}, {'3','C'}, {'3','D'}, {'3','E'}, {'3','F'},
    {'4','0'}, {'4','1'}, {'4','2'}, {'4','3'}, {'4','4'}, {'4','5'}, {'4','6'}, {'4','7'}, {'4','8'}, {'4','9'}, {'4','A'}, {'4','B'}, {'4','C'}, {'4','D'}, {'4','E'}, {'4','F'},
    {'5','0'}, {'5','1'}, {'5','2'}, {'5','3'}, {'5','4'}, {'5','5'}, {'5','6'}, {'5','7'}, {'5','8'}, {'5','9'}, {'5','A'}, {'5','B'}, {'5','C'}, {'5','D'}, {'5','E'}, {'5','F'},
    {'6','0'}, {'6','1'}, {'6','2'}, {'6','3'}, {'6','4'}, {'6','5'}, {'6','6'}, {'6','7'}, {'6','8'}, {'6','9'}, {'6','A'}, {'6','B'}, {'6','C'}, {'6','D'}, {'6','E'}, {'6','F'},
    {'7','0'}, {'7','1'}, {'7','2'}, {'7','3'}, {'7','4'}, {'7','5'}, {'7','6'}, {'7','7'}, {'7','8'}, {'7','9'}, {'7','A'}, {'7','B'}, {'7','C'}, {'7','D'}, {'7','E'}, {'7','F'},
    {'8','0'}, {'8','1'}, {'8','2'}, {'8','3'}, {'8','4'}, {'8','5'}, {'8','6'}, {'8','7'}, {'8','8'}, {'8','9'}, {'8','A'}, {'8','B'}, {'8','C'}, {'8','D'}, {'8','E'}, {'8','F'},
    {'9','0'}, {'9','1'}, {'9','2'}, {'9','3'}, {'9','4'}, {'9','5'}, {'9','6'}, {'9','7'}, {'9','8'}, {'9','9'}, {'9','A'}, {'9','B'}, {'9','C'}, {'9','D'}, {'9','E'}, {'9','F'},
    {'A','0'}, {'A','1'}, {'A','2'}, {'A','3'}, {'A','4'}, {'A','5'}, {'A','6'}, {'A','7'}, {'A','8'}, {'A','9'}, {'A','A'}, {'A','B'}, {'A','C'}, {'A','D'}, {'A','E'}, {'A','F'},
    {'B','0'}, {'B','1'}, {'B','2'}, {'B','3'}, {'B','4'}, {'B','5'}, {'B','6'}, {'B','7'}, {'B','8'}, {'B','9'}, {'B','A'}, {'B','B'}, {'B','C'}, {'B','D'}, {'B','E'}, {'B','F'},
    {'C','0'}, {'C','1'}, {'C','2'}, {'C','3'}, {'C','4'}, {'C','5'}, {'C','6'}, {'C','7'}, {'C','8'}, {'C','9'}, {'C','A'}, {'C','B'}, {'C','C'}, {'C','D'}, {'C','E'}, {'C','F'},
    {'D','0'}, {'D','1'}, {'D','2'}, {'D','3'}, {'D','4'}, {'D','5'}, {'D','6'}, {'D','7'}, {'D','8'}, {'D','9'}, {'D','A'}, {'D','B'}, {'D','C'}, {'D','D'}, {'D','E'}, {'D','F'},
    {'E','0'}, {'E','1'}, {'E','2'}, {'E','3'}, {'E','4'}, {'E','5'}, {'E','6'}, {'E','7'}, {'E','8'}, {'E','9'}, {'E','A'}, {'E','B'}, {'E','C'}, {'E','D'}, {'E','E'}, {'E','F'},
    {'F','0'}, {'F','1'}, {'F','2'}, {'F','3'}, {'F','4'}, {'F','5'}, {'F','6'}, {'F','7'}, {'F','8'}, {'F','9'}, {'F','A'}, {'F','B'}, {'F','C'}, {'F','D'}, {'F','E'}, {'F','F'},
};

void XS_StrToHex(void * dst, const void * str, size_t len)
{
    unsigned char * wcurr = (unsigned char *)dst;
    const unsigned char * rcurr = (const unsigned char *)str;
    const unsigned char * end = rcurr + len;
    unsigned char hoff;
    for (; rcurr < end ; ++rcurr) {
        hoff = *rcurr ;
        *(wcurr++) = (HTBL_UPPER [hoff][0]);
        *(wcurr++) = (HTBL_UPPER [hoff][1]);
    }
}

void XS_HexToStr(void * dst, const void * src, size_t len)
{
    const unsigned char * s = (const unsigned char*)src;
    unsigned char * d = (unsigned char *)dst;
    size_t mask = 1;
    len &= ~mask;
    unsigned char uc0, uc1;
    for(size_t i = 0 ; i < len ; i += 2) {
        uc0 = s[i];
        uc1 = s[i+1];
        *(d++) = ((CTBL[uc0] << 4) + CTBL[uc1]);
    }
}
