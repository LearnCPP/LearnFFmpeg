#pragma once

#include<vector>
#include "Channel.h"

// CShowVideo 对话框

class CShowVideo : public CDialog
{
	DECLARE_DYNAMIC(CShowVideo)

public:
	CShowVideo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CShowVideo();

// 对话框数据
	enum { IDD = IDD_VIDEO_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
protected:
	std::vector<CChannel*> m_vtrChannel;
	CString m_strLog; //日志格式化
	int m_bFirst;
	BOOL m_bFullScreen;
	CRect m_videoRect; //保存窗口大小
	HWND  m_hParentWnd;

	CBrush m_brush; //背景画刷
	BOOL m_bSelected; //是否选择状态
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
