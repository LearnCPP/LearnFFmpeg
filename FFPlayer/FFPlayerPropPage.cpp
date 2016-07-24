// FFPlayerPropPage.cpp : CFFPlayerPropPage ����ҳ���ʵ�֡�

#include "stdafx.h"
#include "FFPlayer.h"
#include "FFPlayerPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CFFPlayerPropPage, COlePropertyPage)

// ��Ϣӳ��

BEGIN_MESSAGE_MAP(CFFPlayerPropPage, COlePropertyPage)
END_MESSAGE_MAP()

// ��ʼ���๤���� guid

IMPLEMENT_OLECREATE_EX(CFFPlayerPropPage, "FFPLAYER.FFPlayerPropPage.1",
	0x51c9217f, 0xa7f4, 0x46ae, 0xae, 0x16, 0x20, 0x7, 0x1a, 0x5c, 0x89, 0xd5)

// CFFPlayerPropPage::CFFPlayerPropPageFactory::UpdateRegistry -
// ��ӻ��Ƴ� CFFPlayerPropPage ��ϵͳע�����

BOOL CFFPlayerPropPage::CFFPlayerPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_FFPLAYER_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}

// CFFPlayerPropPage::CFFPlayerPropPage - ���캯��

CFFPlayerPropPage::CFFPlayerPropPage() :
	COlePropertyPage(IDD, IDS_FFPLAYER_PPG_CAPTION)
{
}

// CFFPlayerPropPage::DoDataExchange - ��ҳ�����Լ��ƶ�����

void CFFPlayerPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}

// CFFPlayerPropPage ��Ϣ�������
