#pragma once

// FFPlayerPropPage.h : CFFPlayerPropPage ����ҳ���������


// CFFPlayerPropPage : �й�ʵ�ֵ���Ϣ������� FFPlayerPropPage.cpp��

class CFFPlayerPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CFFPlayerPropPage)
	DECLARE_OLECREATE_EX(CFFPlayerPropPage)

// ���캯��
public:
	CFFPlayerPropPage();

// �Ի�������
	enum { IDD = IDD_PROPPAGE_FFPLAYER };

// ʵ��
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ��Ϣӳ��
protected:
	DECLARE_MESSAGE_MAP()
};

