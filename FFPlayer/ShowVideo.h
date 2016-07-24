#pragma once

#include<vector>
#include "Channel.h"

// CShowVideo �Ի���

class CShowVideo : public CDialog
{
	DECLARE_DYNAMIC(CShowVideo)

public:
	CShowVideo(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CShowVideo();

// �Ի�������
	enum { IDD = IDD_VIDEO_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
protected:
	std::vector<CChannel*> m_vtrChannel;
	CString m_strLog; //��־��ʽ��
	int m_bFirst;
	BOOL m_bFullScreen;
	CRect m_videoRect; //���洰�ڴ�С
	HWND  m_hParentWnd;

	CBrush m_brush; //������ˢ
	BOOL m_bSelected; //�Ƿ�ѡ��״̬
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
	void Play(LPCTSTR strIp, LONG nPort, SHORT nNum);
	void Stop(void);
	void SetHWND(HWND hWnd){ m_hParentWnd = hWnd; }
	void SetSelected(BOOL bSel){ m_bSelected = bSel; Invalidate(); }
	virtual BOOL DestroyWindow();
private:
	virtual BOOL OnInitDialog();
		
};
