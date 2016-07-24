#pragma once

#include "ShowVideo.h"

// FFPlayerCtrl.h : CFFPlayerCtrl ActiveX �ؼ����������


// CFFPlayerCtrl : �й�ʵ�ֵ���Ϣ������� FFPlayerCtrl.cpp��

class CFFPlayerCtrl : public COleControl
{
	DECLARE_DYNCREATE(CFFPlayerCtrl)

// ���캯��
public:
	CFFPlayerCtrl();

// ��д
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();

// ʵ��
protected:
	~CFFPlayerCtrl();

	DECLARE_OLECREATE_EX(CFFPlayerCtrl)    // �๤���� guid
	DECLARE_OLETYPELIB(CFFPlayerCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CFFPlayerCtrl)     // ����ҳ ID
	DECLARE_OLECTLTYPE(CFFPlayerCtrl)		// �������ƺ�����״̬

// ��Ϣӳ��
	DECLARE_MESSAGE_MAP()

// ����ӳ��
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// �¼�ӳ��
	DECLARE_EVENT_MAP()

// ���Ⱥ��¼� ID
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
	CString m_strLog; //��־��ʽ��
	CShowVideo *m_pVideoDlg; //��ʾ��Ƶ�Ի���
	virtual void OnSetClientSite();
	
};

