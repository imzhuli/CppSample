#pragma once
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct { char __PlaceHolder__ [1]; } * XelString;

extern XelString      XS_New(void);
extern XelString      XS_Duplicate(XelString Str);
extern XelString      XS_NewData(const char * SourcePtr, size_t Length);
extern XelString      XS_NewString(const char * StringPtr);
extern void           XS_Clear(XelString Str);
extern void           XS_Free(XelString Str);
extern const char *   XS_GetData(XelString Str);
extern size_t         XS_GetLength(XelString Str);
extern void           XS_SetData(XelString Str, const char * SourcePtr, size_t Length);
extern void           XS_SetString(XelString Str, const char * StringPtr);
extern void           XS_Append(XelString Str, XelString TailStr);
extern void           XS_AppendData(XelString Str, const char * SourcePtr, size_t Length);
extern void           XS_AppendString(XelString Str, const char * StringPtr);
extern XelString      XS_SubString(XelString Str, size_t Index, size_t Length);
extern XelString      XS_Concat(XelString Str1, XelString Str2);
extern XelString      XS_HexShow(const void * DataPtr, size_t Length, bool NeedHeader);
extern void           XS_PrintHexShow(FILE * stream, const void * DataPtr, size_t Length, bool NeedHeader);
extern void           XS_StrToHex(void * dst, const void * str, size_t len);
extern void           XS_HexToStr(void * dst, const void * str, size_t len);

#ifdef __cplusplus
}
#endif
