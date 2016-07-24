#pragma once

#include "ShowVideo.h"

// FFPlayerCtrl.h : CFFPlayerCtrl ActiveX 控件类的声明。


// CFFPlayerCtrl : 有关实现的信息，请参阅 FFPlayerCtrl.cpp。

class CFFPlayerCtrl : public COleControl
{
	DECLARE_DYNCREATE(CFFPlayerCtrl)

// 构造函数
public:
	CFFPlayerCtrl();

// 重写
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

// 实现
protected:
	~CFFPlayerCtrl();

	DECLARE_OLECREATE_EX(CFFPlayerCtrl)    // 类工厂和 guid
	DECLARE_OLETYPELIB(CFFPlayerCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CFFPlayerCtrl)     // 属性页 ID
	DECLARE_OLECTLTYPE(CFFPlayerCtrl)		// 类型名称和杂项状态

// 消息映射
	DECLARE_MESSAGE_MAP()

// 调度映射
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// 事件映射
	DECLARE_EVENT_MAP()

// 调度和事件 ID
public:
	enum {
		dispidSetChannel = 2L,
		dispidStart = 1L
	};

protected:
	void Start(LPCTSTR strUrl, LONG nPort, LONG nNum);
	void SetChannel(VARIANT_BOOL bSelected);

public:
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
public:
	CString m_strLog; //日志格式化
	CShowVideo *m_pVideoDlg; //显示视频对话框
	virtual void OnSetClientSite();
	
};

