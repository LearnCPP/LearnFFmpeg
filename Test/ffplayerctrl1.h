#pragma once

// 计算机生成了由 Microsoft Visual C++ 创建的 IDispatch 包装器类

// 注意:  不要修改此文件的内容。  如果此类由
//  Microsoft Visual C++ 重新生成，您的修改将被覆盖。

/////////////////////////////////////////////////////////////////////////////
// CFfplayerctrl1 包装器类

class CFfplayerctrl1 : public CWnd
{
protected:
	DECLARE_DYNCREATE(CFfplayerctrl1)
public:
	CLSID const& GetClsid()
	{
		static CLSID const clsid
			= { 0x81997F5C, 0x2444, 0x4106, { 0xAE, 0x8D, 0x16, 0x7F, 0x47, 0xF7, 0xC, 0x57 } };
		return clsid;
	}
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); 
	}

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, 
				UINT nID, CFile* pPersist = NULL, BOOL bStorage = FALSE,
				BSTR bstrLicKey = NULL)
	{ 
		return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
		pPersist, bStorage, bstrLicKey); 
	}

// 特性
public:


// 操作
public:

// _DFFPlayer

// Functions
//

	void AboutBox()
	{
		InvokeHelper(DISPID_ABOUTBOX, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
	}
	void Start(LPCTSTR strUrl, long nPort, long nNum)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 VTS_I4 ;
		InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms, strUrl, nPort, nNum);
	}
	void SetChannel(BOOL bSelected)
	{
		static BYTE parms[] = VTS_BOOL ;
		InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bSelected);
	}

// Properties
//



};
