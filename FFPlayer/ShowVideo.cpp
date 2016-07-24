// ShowVideo.cpp : 实现文件
//

#include "stdafx.h"
#include "FFPlayer.h"
#include "ShowVideo.h"
#include "afxdialogex.h"
#include "WinLog.h"

#define ID_DISPALYTIME 1  //定时器ID
#define ID_DEODER_RESTART_TIME 2 //检测解码器是否需要重启

// CShowVideo 对话框

IMPLEMENT_DYNAMIC(CShowVideo, CDialog)

CShowVideo::CShowVideo(CWnd* pParent /*=NULL*/)
	: CDialog(CShowVideo::IDD, pParent)
	, m_bFullScreen(FALSE)
	, m_hParentWnd(NULL)
	, m_bSelected(FALSE)
{
//	m_strLog.Format(_T("CShowVideo 构造，pParent =%x"), pParent);
//	WriteRunLog(m_strLog);
	m_brush.CreateSolidBrush(RGB(0, 0, 0));
}

CShowVideo::~CShowVideo()
{
//	m_strLog.Format(_T("CShowVideo 析造"));
//	WriteRunLog(m_strLog);
}

void CShowVideo::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShowVideo, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CShowVideo 消息处理程序
void CShowVideo::OnLButtonDown(UINT nFlags, CPoint point)
{
	for (auto it = m_vtrChannel.begin(); it != m_vtrChannel.end(); ++it)
	{
		if ((*it)->GetAudioPlay() && (*it)->IsInChannelRect(point))
		{//判断已经在播放声音的节目，则关闭
			(*it)->SetAudioPlay(FALSE);
			break;
		}
		(*it)->SetAudioPlay((*it)->IsInChannelRect(point));
	}

//	InvalidateRect(NULL);

	CDialog::OnLButtonDown(nFlags, point);
}


void CShowVideo::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	
#ifndef FULLSCREEN 
	if (!m_bFullScreen)
	{
		for (auto it = m_vtrChannel.begin(); it != m_vtrChannel.end(); ++it)
		{
			if ((*it)->IsInChannelRect(point))
			{//找到需要全屏的channel
				(*it)->SetFullScreen(TRUE);
				(*it)->SetAudioPlay(TRUE);
				break;
			}
		}

		GetWindowRect(&m_videoRect);
		::SetParent(GetSafeHwnd(), ::GetDesktopWindow());
		int cx = ::GetSystemMetrics(SM_CXSCREEN);
		int cy = ::GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(0, 0, cx, cy);
		SetFocus();
		::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, cx, cy, SWP_FRAMECHANGED | SWP_DEFERERASE);
		m_bFullScreen = TRUE;
	}
	else{
		for (auto it = m_vtrChannel.begin(); it != m_vtrChannel.end(); ++it)
		{
			if ((*it)->GetFullScreen())
			{//找到全屏的channel,并设置成FALSE
				(*it)->SetFullScreen(FALSE);
				(*it)->SetAudioPlay((*it)->GetAudioPlayBeforeFull());
				break;
			}
		}
		::SetParent(GetSafeHwnd(), m_hParentWnd);
		ScreenToClient(&m_videoRect);
		MoveWindow(&m_videoRect);
		::SetWindowPos(GetSafeHwnd(), HWND_NOTOPMOST, m_videoRect.left, m_videoRect.top,
			m_videoRect.Width(), m_videoRect.Height(), SWP_NOMOVE | SWP_NOSIZE);
		m_bFullScreen = FALSE;
	}
#endif

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CShowVideo::Play(LPCTSTR strIp, LONG nPort, SHORT nNum)
{
	CRect rect;
	GetClientRect(&rect);
	CString url(strIp);
	int nBasePort = nPort;
	int nLoop = (int)sqrt((float)nNum);

	//
	m_strLog.Format(_T("控件Play,正在运行，rect=%d,%d,%d,%d"), rect.left, rect.right, rect.Width(),
		rect.Height());
	CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);

	int xGap = 0, yGap = 0;
	int width = rect.Width()-0;
	int height = rect.Height()-0;
	int x = rect.left + xGap;
	int y = rect.top + yGap;
	int video_width, audio_width = 40, video_height, audio_height;
	video_width = (width - (audio_width*nLoop + xGap*(nLoop + 1))) / nLoop;
	video_height = audio_height = (height - yGap*(nLoop + 1)) / nLoop;

	for (int i = 0; i < nLoop; ++i)
		for (int j = 0; j < nLoop; ++j)
		{
			CChannel* pchannel = NULL;

			pchannel = new CChannel(GetSafeHwnd()/*, i*/);

			int video_x = x + i%nLoop*(video_width + audio_width + xGap);
			int video_y = y + (i + j) % nLoop*(video_height + yGap);
			int audio_x = x + video_width + i%nLoop*(video_width + audio_width + xGap);
			int audio_y = video_y;
			pchannel->putVideoOrAudioRect(video_x, video_y, video_width, video_height, TRUE);//设置视频位置
			pchannel->putVideoOrAudioRect(audio_x, audio_y, audio_width, audio_height, FALSE);//设置音频位置
#if _DEBUG
			m_strLog.Format(_T("视频频位置，video_x：%d, video_y：%d, video_width：%d, video_height：%d"),
				video_x, video_y, video_width, video_height);
			CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);
			m_strLog.Format(_T("音频位置，audio_x：%d, audio_y：%d, audio_width：%d, audio_height：%d"),
				audio_x, audio_y, audio_width, audio_height);
			CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);
#endif
			
			BOOL bRet = pchannel->InitAV(url, nBasePort);
			if (!bRet)
			{
				delete pchannel;
				m_strLog.Format(_T("%s:%d 初始化InitAV失败"), url, nBasePort);
				CWinLog::GetInstance()->WriteLog(m_strLog, emErrorLog);
				continue;
			}
			//初始化成功，加入CChannel数组中
			m_vtrChannel.push_back(pchannel);
			++nBasePort;
		}
	m_strLog.Format(_T("控件Play,运行完毕，Channel size=%d"), m_vtrChannel.size());
	CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);
}

void CShowVideo::Stop(void)
{
	KillTimer(ID_DISPALYTIME);
	KillTimer(ID_DEODER_RESTART_TIME);

	for (auto it = m_vtrChannel.begin(); it != m_vtrChannel.end(); ++it)
	{
		if (*it)
		{
			delete (*it);
		}
	}
	m_vtrChannel.clear();
}


BOOL CShowVideo::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
//	SetTimer(ID_DISPALYTIME, 1000 / 20, NULL);
	SetTimer(ID_DEODER_RESTART_TIME, 2 * 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CShowVideo::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ID_DISPALYTIME)
	{
		for (auto it = m_vtrChannel.begin(); it != m_vtrChannel.end(); ++it)
		{
			//(*it)->DrawVideo();
		}
	}
	else if (nIDEvent == ID_DEODER_RESTART_TIME)
	{
		for (auto it = m_vtrChannel.begin(); it != m_vtrChannel.end(); ++it)
		{
			(*it)->DetecteDecoderRestart();
		}
	}

	CDialog::OnTimer(nIDEvent);
}


BOOL CShowVideo::DestroyWindow()
{
	// TODO:  在此添加专用代码和/或调用基类
	m_strLog.Format(_T(" CShowVideo::DestroyWindow"));
	WriteRunLog(m_strLog);
	return CDialog::DestroyWindow();
}


BOOL CShowVideo::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
// 	if (m_bSelected)
// 	{
// 		CBrush *brush = CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH));
// 		CBrush * pOldBrush = pDC->SelectObject(brush);
// 
// 		CPen pen(PS_SOLID, 2, RGB(0, 255, 0));
// 		CPen* pOldPen = pDC->SelectObject(&pen);
// 
// 		CRect rect;
// 		GetClientRect(&rect);
// 		pDC->Rectangle(rect);
// 
// 		pDC->SelectObject(pOldBrush);
// 		pDC->SelectObject(pOldPen);
// 	}
// 	
// 	return TRUE;
	return CDialog::OnEraseBkgnd(pDC);
}
