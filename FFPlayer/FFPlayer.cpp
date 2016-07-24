// FFPlayer.cpp : CFFPlayerApp 和 DLL 注册的实现。

#include "stdafx.h"
#include "FFPlayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CFFPlayerApp theApp;

const GUID CDECL _tlid = { 0x68E4E07, 0xE348, 0x44A0, { 0x89, 0xA4, 0xAE, 0x86, 0x3A, 0xC2, 0x3D, 0x3C } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;



// CFFPlayerApp::InitInstance - DLL 初始化

BOOL CFFPlayerApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO:  在此添加您自己的模块初始化代码。
	}

	return bInit;
}



// CFFPlayerApp::ExitInstance - DLL 终止

int CFFPlayerApp::ExitInstance()
{
	// TODO:  在此添加您自己的模块终止代码。

	return COleControlModule::ExitInstance();
}



// DllRegisterServer - 将项添加到系统注册表

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}



// DllUnregisterServer - 将项从系统注册表中移除

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}
