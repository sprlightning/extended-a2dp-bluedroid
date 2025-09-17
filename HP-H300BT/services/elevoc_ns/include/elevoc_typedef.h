/*
 * =============================================================================
 * Copyright (c) 2017-2021 Elevoc Technologies, Inc.
 * All rights reserved. Elevoc Proprietary and Confidential.
 * =============================================================================
 */

#ifndef ELEVOC_TYPEDEF_H
#define ELEVOC_TYPEDEF_H

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#if defined(__ARMCC_VERSION)
#define ELEVOC_INLINE          __attribute__((inline))
#define ELEVOC_NO_INLINE         __attribute__((noinline))
#else
#define ELEVOC_INLINE
#define ELEVOC_NO_INLINE
#endif

/* signed char type defined */
typedef char                    ele_char;
/* signed integer type defined */
typedef char                    ele_int8;
typedef short                   ele_int16;
typedef int                     ele_int32;
typedef long long               ele_int64;

/* unsigned signed integer type defined */
typedef unsigned char           ele_uint8;
typedef unsigned short          ele_uint16;
typedef unsigned int            ele_uint32;
typedef unsigned long long      ele_uint64;

/* float type defined */
typedef float                   ele_float;
typedef double                  ele_double;

/* complex type defined */
typedef union                   _ele_cint16
{
    ele_int32        i32;
    struct
    {
        ele_int16    real;
        ele_int16    imag;
    }i16;
}ele_cint16;

typedef union                   _ele_cint32
{
    ele_int64        i64;
    struct
    {
        ele_int32    real;
        ele_int32    imag;
    }i32;
}ele_cint32;

typedef struct {
    ele_int32 real;
    ele_int32 imag;
}ele_cfix32;

typedef struct                  _ele_cfloat
{
    ele_float        real;
    ele_float        imag;
}ele_cfloat;


typedef struct _ele_packet
{
    ele_uint32      id;
    ele_uint32      size;
    void            *data;
}ele_packet;


/* error enum type defined */
typedef ele_uint16              ele_error;
#define ELE_NO_ERROR            0x0000
#define ELE_AUTH_ERROR          0x0001
#define ELE_STREAM_ERROR        0x0002
#define ELE_MEMORY_ERROR        0x0004
#define ELE_MODEL_ERROR         0x0008
#define ELE_PARAM_ERROR         0x0010
#define ELE_SERVICE_ERROR       0x0020
#define ELE_THREAD_ERROR        0x0040
#define ELE_FILE_ERROR          0x0080
#define ELE_UNDEF_ERROR         0xFFFF

/* data enum type defined */
typedef ele_uint8               ele_dtype;
#define ELE_CHAR                (0x00|sizeof(ele_char))
#define ELE_INT8                (0x10|sizeof(ele_int8))
#define ELE_UINT8               (0x20|sizeof(ele_uint8))

#define ELE_INT16               (0x00|sizeof(ele_int16))
#define ELE_UINT16              (0x10|sizeof(ele_uint16))

#define ELE_INT32               (0x00|sizeof(ele_int32))
#define ELE_UINT32              (0x10|sizeof(ele_uint32))
#define ELE_FLOAT               (0x20|sizeof(ele_float))
#define ELE_CINT16              (0x30|sizeof(ele_cint16))

#define ELE_INT64               (0x00|sizeof(ele_int64))
#define ELE_UINT64              (0x10|sizeof(ele_uint64))
#define ELE_DOUBLE              (0x20|sizeof(ele_double))
#define ELE_CINT32              (0x30|sizeof(ele_cint32))
#define ELE_CFLOAT              (0x40|sizeof(ele_cfloat))

typedef ele_int32               ele_band_mode;
#define ELE_BM_8000             0x0
#define ELE_BM_16000            0x1
#define ELE_BM_32000            0x2
#define ELE_BM_48000            0x3

/* data type sizeof compute macro */
#define dtype_sizeof(x)         ((x)&0x0f)

/* math definition */
#define ELE_PI                  3.14159265358979

/* integer data type limited defined */
#define ELE_MIN_S8      (-127 - 1)
#define ELE_MAX_S8      127
#define ELE_MAX_U8      0xffu

#define ELE_MIN_S16     (ele_int16)-32768 /* 0x8000 */
#define ELE_MAX_S16     (ele_int16)+32767 /* 0x7fff */
#define ELE_MAX_U16     (ele_uint16)0xffffu

#define ELE_MIN_S32     (ele_int32)0x80000000L
#define ELE_MAX_S32     (ele_int32)0x7fffffffL
#define ELE_MAX_U32     (ele_uint32)0xffffffffU

#define ELE_MIN_S64     (ele_int64)0x8000000000000000LL
#define ELE_MAX_S64     (ele_int64)0x7fffffffffffffffLL
#define ELE_MAX_U64     (ele_uint64)0xffffffffffffffffULL

/* data type convert macro between float type and fix type */
#define float2fix(x, Q)         (ele_int32)(((ele_float)x)*(ele_float)(1ULL << (Q)))
#define fix2float(x, Q)         ((ele_float)(x)/(ele_float)(1UL << (Q)))

#define double2fix(x, Q)         ((x)*(ele_double)(1ULL << (Q)))
#define fix2double(x, Q)         ((ele_double)(x)/(1UL << (Q)))

#endif  //ELEVOC_TYPEDEF_H
