#pragma once

// FFPlayerPropPage.h : CFFPlayerPropPage 属性页类的声明。


// CFFPlayerPropPage : 有关实现的信息，请参阅 FFPlayerPropPage.cpp。

class CFFPlayerPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CFFPlayerPropPage)
	DECLARE_OLECREATE_EX(CFFPlayerPropPage)

// 构造函数
public:
	CFFPlayerPropPage();

// 对话框数据
	enum { IDD = IDD_PROPPAGE_FFPLAYER };

// 实现
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 消息映射
protected:
	DECLARE_MESSAGE_MAP()
};

