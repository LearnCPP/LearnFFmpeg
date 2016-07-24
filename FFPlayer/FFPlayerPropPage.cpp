// FFPlayerPropPage.cpp : CFFPlayerPropPage 属性页类的实现。

#include "stdafx.h"
#include "FFPlayer.h"
#include "FFPlayerPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CFFPlayerPropPage, COlePropertyPage)

// 消息映射

BEGIN_MESSAGE_MAP(CFFPlayerPropPage, COlePropertyPage)
END_MESSAGE_MAP()

// 初始化类工厂和 guid

IMPLEMENT_OLECREATE_EX(CFFPlayerPropPage, "FFPLAYER.FFPlayerPropPage.1",
	0x51c9217f, 0xa7f4, 0x46ae, 0xae, 0x16, 0x20, 0x7, 0x1a, 0x5c, 0x89, 0xd5)

// CFFPlayerPropPage::CFFPlayerPropPageFactory::UpdateRegistry -
// 添加或移除 CFFPlayerPropPage 的系统注册表项

BOOL CFFPlayerPropPage::CFFPlayerPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_FFPLAYER_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}

// CFFPlayerPropPage::CFFPlayerPropPage - 构造函数

CFFPlayerPropPage::CFFPlayerPropPage() :
	COlePropertyPage(IDD, IDS_FFPLAYER_PPG_CAPTION)
{
}

// CFFPlayerPropPage::DoDataExchange - 在页和属性间移动数据

void CFFPlayerPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}

// CFFPlayerPropPage 消息处理程序
