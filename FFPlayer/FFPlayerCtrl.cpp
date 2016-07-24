// FFPlayerCtrl.cpp : CFFPlayerCtrl ActiveX 控件类的实现。

#include "stdafx.h"
#include "FFPlayer.h"
#include "FFPlayerCtrl.h"
#include "FFPlayerPropPage.h"
#include "afxdialogex.h"

#include "WinLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CFFPlayerCtrl, COleControl)

// 消息映射

BEGIN_MESSAGE_MAP(CFFPlayerCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// 调度映射

BEGIN_DISPATCH_MAP(CFFPlayerCtrl, COleControl)
	DISP_FUNCTION_ID(CFFPlayerCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CFFPlayerCtrl, "Start", dispidStart, Start, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4)
	DISP_FUNCTION_ID(CFFPlayerCtrl, "SetChannel", dispidSetChannel, SetChannel, VT_EMPTY, VTS_BOOL)
END_DISPATCH_MAP()

// 事件映射

BEGIN_EVENT_MAP(CFFPlayerCtrl, COleControl)
END_EVENT_MAP()

// 属性页

// TODO:  按需要添加更多属性页。  请记住增加计数!
BEGIN_PROPPAGEIDS(CFFPlayerCtrl, 1)
	PROPPAGEID(CFFPlayerPropPage::guid)
END_PROPPAGEIDS(CFFPlayerCtrl)

// 初始化类工厂和 guid

IMPLEMENT_OLECREATE_EX(CFFPlayerCtrl, "FFPLAYER.FFPlayerCtrl.1",
	0x81997f5c, 0x2444, 0x4106, 0xae, 0x8d, 0x16, 0x7f, 0x47, 0xf7, 0xc, 0x57)

// 键入库 ID 和版本

IMPLEMENT_OLETYPELIB(CFFPlayerCtrl, _tlid, _wVerMajor, _wVerMinor)

// 接口 ID

const IID IID_DFFPlayer = { 0x7307DBF7, 0x2885, 0x4DB9, { 0xBE, 0x54, 0x65, 0x20, 0x66, 0xC5, 0xBF, 0xB0 } };
const IID IID_DFFPlayerEvents = { 0x456F141F, 0x72AC, 0x472E, { 0x87, 0x58, 0xD3, 0x6C, 0x47, 0x40, 0xEE, 0x34 } };

// 控件类型信息

static const DWORD _dwFFPlayerOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CFFPlayerCtrl, IDS_FFPLAYER, _dwFFPlayerOleMisc)

// CFFPlayerCtrl::CFFPlayerCtrlFactory::UpdateRegistry -
// 添加或移除 CFFPlayerCtrl 的系统注册表项

BOOL CFFPlayerCtrl::CFFPlayerCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO:  验证您的控件是否符合单元模型线程处理规则。
	// 有关更多信息，请参考 MFC 技术说明 64。
	// 如果您的控件不符合单元模型规则，则
	// 必须修改如下代码，将第六个参数从
	// afxRegApartmentThreading 改为 0。

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_FFPLAYER,
			IDB_FFPLAYER,
			afxRegApartmentThreading,
			_dwFFPlayerOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


// CFFPlayerCtrl::CFFPlayerCtrl - 构造函数

CFFPlayerCtrl::CFFPlayerCtrl()
	:m_strLog(_T(""))
{
	InitializeIIDs(&IID_DFFPlayer, &IID_DFFPlayerEvents);
	// TODO:  在此初始化控件的实例数据。
	FF_Global_init();
}

// CFFPlayerCtrl::~CFFPlayerCtrl - 析构函数

CFFPlayerCtrl::~CFFPlayerCtrl()
{
	// TODO:  在此清理控件的实例数据。
	FF_Global_unint();
	
	CWinLog::ReleaseInstance();
}

// CFFPlayerCtrl::OnDraw - 绘图函数

void CFFPlayerCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& /* rcInvalid */)
{
	if (!pdc)
		return;

	// TODO:  用您自己的绘图代码替换下面的代码。
// 	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
// 	pdc->Ellipse(rcBounds);
}

// CFFPlayerCtrl::DoPropExchange - 持久性支持

void CFFPlayerCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO:  为每个持久的自定义属性调用 PX_ 函数。
}


// CFFPlayerCtrl::OnResetState - 将控件重置为默认状态

void CFFPlayerCtrl::OnResetState()
{
	COleControl::OnResetState();  // 重置 DoPropExchange 中找到的默认值

	// TODO:  在此重置任意其他控件状态。
}


// CFFPlayerCtrl::AboutBox - 向用户显示“关于”框

void CFFPlayerCtrl::AboutBox()
{
	CDialogEx dlgAbout(IDD_ABOUTBOX_FFPLAYER);
	dlgAbout.DoModal();
}


// CFFPlayerCtrl 消息处理程序



void CFFPlayerCtrl::Start(LPCTSTR strUrl, LONG nPort, LONG nNum)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO:  在此添加调度处理程序代码
	m_pVideoDlg->Play(strUrl, nPort, nNum);
}

void CFFPlayerCtrl::OnSize(UINT nType, int cx, int cy)
{
	COleControl::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码

	m_strLog.Format(_T("cx =%d; cy = %d"), cx, cy);
	CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);
	m_pVideoDlg->MoveWindow(CRect(0, 0, cx, cy));

}


void CFFPlayerCtrl::OnDestroy()
{
	COleControl::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
	m_strLog.Format(_T("PlayerPlugIn 控件，正在做资源析构..."));
	WriteRunLog(m_strLog);

	if (m_pVideoDlg)
	{
		m_pVideoDlg->Stop();
		m_pVideoDlg->DestroyWindow();
		delete m_pVideoDlg;
		m_pVideoDlg = NULL;
	}

	m_strLog.Format(_T("PlayerPlugIn 控件，资源析构完毕！ 退出..."));
	WriteRunLog(m_strLog);

}

int CFFPlayerCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_strLog.Format(_T("PlayerPlugIn 控件启动，x=%d,y=%d,cx=%d,cy=%d"), lpCreateStruct->x, lpCreateStruct->y,
		lpCreateStruct->cx, lpCreateStruct->cy);

	CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);

	m_pVideoDlg = new CShowVideo();
	CRect rect;
	GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_pVideoDlg->Create(IDD_VIDEO_DLG);
	::SetParent(m_pVideoDlg->GetSafeHwnd(), GetSafeHwnd());
	m_pVideoDlg->SetHWND(GetSafeHwnd());
	::MoveWindow(m_pVideoDlg->GetSafeHwnd(), rect.left, rect.top, rect.Width(), rect.Height(), TRUE);
	m_pVideoDlg->ShowWindow(SW_SHOWNORMAL);
	m_strLog.Format(_T("m_pVideoDlg->GetSafeHwnd()=%x,"), m_pVideoDlg->GetSafeHwnd());

	CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);

	return 0;
}


void CFFPlayerCtrl::OnSetClientSite()
{
	// TODO:  在此添加专用代码和/或调用基类

	COleControl::OnSetClientSite();
}


void CFFPlayerCtrl::SetChannel(VARIANT_BOOL bSelected)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO:  在此添加调度处理程序代码
	m_pVideoDlg->SetSelected(bSelected);
}
