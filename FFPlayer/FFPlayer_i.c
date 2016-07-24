

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Sat May 14 10:48:39 2016
 */
/* Compiler settings for FFPlayer.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, LIBID_FFPlayerLib,0x068E4E07,0xE348,0x44A0,0x89,0xA4,0xAE,0x86,0x3A,0xC2,0x3D,0x3C);


MIDL_DEFINE_GUID(IID, DIID__DFFPlayer,0x7307DBF7,0x2885,0x4DB9,0xBE,0x54,0x65,0x20,0x66,0xC5,0xBF,0xB0);


MIDL_DEFINE_GUID(IID, DIID__DFFPlayerEvents,0x456F141F,0x72AC,0x472E,0x87,0x58,0xD3,0x6C,0x47,0x40,0xEE,0x34);


MIDL_DEFINE_GUID(CLSID, CLSID_FFPlayer,0x81997F5C,0x2444,0x4106,0xAE,0x8D,0x16,0x7F,0x47,0xF7,0x0C,0x57);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



