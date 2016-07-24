

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __FFPlayeridl_h__
#define __FFPlayeridl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef ___DFFPlayer_FWD_DEFINED__
#define ___DFFPlayer_FWD_DEFINED__
typedef interface _DFFPlayer _DFFPlayer;
#endif 	/* ___DFFPlayer_FWD_DEFINED__ */


#ifndef ___DFFPlayerEvents_FWD_DEFINED__
#define ___DFFPlayerEvents_FWD_DEFINED__
typedef interface _DFFPlayerEvents _DFFPlayerEvents;
#endif 	/* ___DFFPlayerEvents_FWD_DEFINED__ */


#ifndef __FFPlayer_FWD_DEFINED__
#define __FFPlayer_FWD_DEFINED__

#ifdef __cplusplus
typedef class FFPlayer FFPlayer;
#else
typedef struct FFPlayer FFPlayer;
#endif /* __cplusplus */

#endif 	/* __FFPlayer_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __FFPlayerLib_LIBRARY_DEFINED__
#define __FFPlayerLib_LIBRARY_DEFINED__

/* library FFPlayerLib */
/* [control][version][uuid] */ 


EXTERN_C const IID LIBID_FFPlayerLib;

#ifndef ___DFFPlayer_DISPINTERFACE_DEFINED__
#define ___DFFPlayer_DISPINTERFACE_DEFINED__

/* dispinterface _DFFPlayer */
/* [uuid] */ 


EXTERN_C const IID DIID__DFFPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("7307DBF7-2885-4DB9-BE54-652066C5BFB0")
    _DFFPlayer : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DFFPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DFFPlayer * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DFFPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DFFPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DFFPlayer * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DFFPlayer * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DFFPlayer * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DFFPlayer * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DFFPlayerVtbl;

    interface _DFFPlayer
    {
        CONST_VTBL struct _DFFPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DFFPlayer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DFFPlayer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DFFPlayer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DFFPlayer_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DFFPlayer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DFFPlayer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DFFPlayer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DFFPlayer_DISPINTERFACE_DEFINED__ */


#ifndef ___DFFPlayerEvents_DISPINTERFACE_DEFINED__
#define ___DFFPlayerEvents_DISPINTERFACE_DEFINED__

/* dispinterface _DFFPlayerEvents */
/* [uuid] */ 


EXTERN_C const IID DIID__DFFPlayerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("456F141F-72AC-472E-8758-D36C4740EE34")
    _DFFPlayerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _DFFPlayerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DFFPlayerEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DFFPlayerEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DFFPlayerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DFFPlayerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DFFPlayerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DFFPlayerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DFFPlayerEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _DFFPlayerEventsVtbl;

    interface _DFFPlayerEvents
    {
        CONST_VTBL struct _DFFPlayerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DFFPlayerEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define _DFFPlayerEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define _DFFPlayerEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define _DFFPlayerEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define _DFFPlayerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define _DFFPlayerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define _DFFPlayerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___DFFPlayerEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_FFPlayer;

#ifdef __cplusplus

class DECLSPEC_UUID("81997F5C-2444-4106-AE8D-167F47F70C57")
FFPlayer;
#endif
#endif /* __FFPlayerLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


