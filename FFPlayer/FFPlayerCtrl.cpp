// FFPlayerCtrl.cpp : CFFPlayerCtrl ActiveX �ؼ����ʵ�֡�

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

// ��Ϣӳ��

BEGIN_MESSAGE_MAP(CFFPlayerCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()

// ����ӳ��

BEGIN_DISPATCH_MAP(CFFPlayerCtrl, COleControl)
	DISP_FUNCTION_ID(CFFPlayerCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
	DISP_FUNCTION_ID(CFFPlayerCtrl, "Start", dispidStart, Start, VT_EMPTY, VTS_BSTR VTS_I4 VTS_I4)
	DISP_FUNCTION_ID(CFFPlayerCtrl, "SetChannel", dispidSetChannel, SetChannel, VT_EMPTY, VTS_BOOL)
END_DISPATCH_MAP()

// �¼�ӳ��

BEGIN_EVENT_MAP(CFFPlayerCtrl, COleControl)
END_EVENT_MAP()

// ����ҳ

// TODO:  ����Ҫ��Ӹ�������ҳ��  ���ס���Ӽ���!
BEGIN_PROPPAGEIDS(CFFPlayerCtrl, 1)
	PROPPAGEID(CFFPlayerPropPage::guid)
END_PROPPAGEIDS(CFFPlayerCtrl)

// ��ʼ���๤���� guid

IMPLEMENT_OLECREATE_EX(CFFPlayerCtrl, "FFPLAYER.FFPlayerCtrl.1",
	0x81997f5c, 0x2444, 0x4106, 0xae, 0x8d, 0x16, 0x7f, 0x47, 0xf7, 0xc, 0x57)

// ����� ID �Ͱ汾

IMPLEMENT_OLETYPELIB(CFFPlayerCtrl, _tlid, _wVerMajor, _wVerMinor)

// �ӿ� ID

const IID IID_DFFPlayer = { 0x7307DBF7, 0x2885, 0x4DB9, { 0xBE, 0x54, 0x65, 0x20, 0x66, 0xC5, 0xBF, 0xB0 } };
const IID IID_DFFPlayerEvents = { 0x456F141F, 0x72AC, 0x472E, { 0x87, 0x58, 0xD3, 0x6C, 0x47, 0x40, 0xEE, 0x34 } };

// �ؼ�������Ϣ

static const DWORD _dwFFPlayerOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CFFPlayerCtrl, IDS_FFPLAYER, _dwFFPlayerOleMisc)

// CFFPlayerCtrl::CFFPlayerCtrlFactory::UpdateRegistry -
// ��ӻ��Ƴ� CFFPlayerCtrl ��ϵͳע�����

BOOL CFFPlayerCtrl::CFFPlayerCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO:  ��֤���Ŀؼ��Ƿ���ϵ�Ԫģ���̴߳������
	// �йظ�����Ϣ����ο� MFC ����˵�� 64��
	// ������Ŀؼ������ϵ�Ԫģ�͹�����
	// �����޸����´��룬��������������
	// afxRegApartmentThreading ��Ϊ 0��

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


// CFFPlayerCtrl::CFFPlayerCtrl - ���캯��

CFFPlayerCtrl::CFFPlayerCtrl()
	:m_strLog(_T(""))
{
	InitializeIIDs(&IID_DFFPlayer, &IID_DFFPlayerEvents);
	// TODO:  �ڴ˳�ʼ���ؼ���ʵ�����ݡ�
	FF_Global_init();
}

// CFFPlayerCtrl::~CFFPlayerCtrl - ��������

CFFPlayerCtrl::~CFFPlayerCtrl()
{
	// TODO:  �ڴ�����ؼ���ʵ�����ݡ�
	FF_Global_unint();
	
	CWinLog::ReleaseInstance();
}

// CFFPlayerCtrl::OnDraw - ��ͼ����

void CFFPlayerCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& /* rcInvalid */)
{
	if (!pdc)
		return;

	// TODO:  �����Լ��Ļ�ͼ�����滻����Ĵ��롣
// 	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
// 	pdc->Ellipse(rcBounds);
}

// CFFPlayerCtrl::DoPropExchange - �־���֧��

void CFFPlayerCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO:  Ϊÿ���־õ��Զ������Ե��� PX_ ������
}


// CFFPlayerCtrl::OnResetState - ���ؼ�����ΪĬ��״̬

void CFFPlayerCtrl::OnResetState()
{
	COleControl::OnResetState();  // ���� DoPropExchange ���ҵ���Ĭ��ֵ

	// TODO:  �ڴ��������������ؼ�״̬��
}


// CFFPlayerCtrl::AboutBox - ���û���ʾ�����ڡ���

void CFFPlayerCtrl::AboutBox()
{
	CDialogEx dlgAbout(IDD_ABOUTBOX_FFPLAYER);
	dlgAbout.DoModal();
}


// CFFPlayerCtrl ��Ϣ�������



void CFFPlayerCtrl::Start(LPCTSTR strUrl, LONG nPort, LONG nNum)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO:  �ڴ���ӵ��ȴ���������
	m_pVideoDlg->Play(strUrl, nPort, nNum);
}

void CFFPlayerCtrl::OnSize(UINT nType, int cx, int cy)
{
	COleControl::OnSize(nType, cx, cy);

	// TODO:  �ڴ˴������Ϣ����������

	m_strLog.Format(_T("cx =%d; cy = %d"), cx, cy);
	CWinLog::GetInstance()->WriteLog(m_strLog, emRunLog);
	m_pVideoDlg->MoveWindow(CRect(0, 0, cx, cy));

}


void CFFPlayerCtrl::OnDestroy()
{
	COleControl::OnDestroy();

	// TODO:  �ڴ˴������Ϣ����������
	m_strLog.Format(_T("PlayerPlugIn �ؼ�����������Դ����..."));
	WriteRunLog(m_strLog);

	if (m_pVideoDlg)
	{
		m_pVideoDlg->Stop();
		m_pVideoDlg->DestroyWindow();
		delete m_pVideoDlg;
		m_pVideoDlg = NULL;
	}

	m_strLog.Format(_T("PlayerPlugIn �ؼ�����Դ������ϣ� �˳�..."));
	WriteRunLog(m_strLog);

}

int CFFPlayerCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	m_strLog.Format(_T("PlayerPlugIn �ؼ�������x=%d,y=%d,cx=%d,cy=%d"), lpCreateStruct->x, lpCreateStruct->y,
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
	// TODO:  �ڴ����ר�ô����/����û���

	COleControl::OnSetClientSite();
}


void CFFPlayerCtrl::SetChannel(VARIANT_BOOL bSelected)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO:  �ڴ���ӵ��ȴ���������
	m_pVideoDlg->SetSelected(bSelected);
}
