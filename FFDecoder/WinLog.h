#pragma once
#include<atlstr.h>

enum EM_LOG
{
	emErrorLog = 1,
	emRunLog,
};

class CWinLog
{
public:
	~CWinLog(void);
private:
	CString m_strExePath; //���г���·��
	CString m_strRunLogName; //������־����
	CString m_strErrorLogName; //������־����
	//const�Ǳ�֤����������󲻻ᱻ�޸ģ�
	//static�Ǳ�֤������������ڳ���ʼʱ����������֮ǰ�ͱ���ʼ���ˣ��������̰߳�ȫ�ġ�
	static const CWinLog* m_sInstance; //��̬��Աʵ����������

public:
	static const CWinLog* GetInstance(void)//����ʵ�����
	{
		if (!m_sInstance)
		{
			m_sInstance = new CWinLog();
		}

		return m_sInstance;
	}
	static void ReleaseInstance()//�ͷ�ʵ�����
	{ 
		if (CWinLog::m_sInstance)
		{
			delete CWinLog::m_sInstance;
			CWinLog::m_sInstance = NULL;
		}
			
	}
	CString GetExePath();
	bool WriteLog(const LPCTSTR& szLog, const LPCTSTR& szFile = NULL,
		const int& line = -1, const int& logtype=emErrorLog) const;
	
private:
	CWinLog(const CString& szLogName = _T("RunLog.log"), const CString& szErrorLogName =_T("ErrorLog.log"));
	CWinLog(CWinLog&);
	bool WriteLogInternal(const LPCTSTR& szLog, const LPCTSTR& szFile,
		const int& line, const LPCTSTR& szLogName)const;
};

#define WriteErrorLog(T) CWinLog::GetInstance()->WriteLog((T),__FILE__,__LINE__)
#define WriteRunLog(T) CWinLog::GetInstance()->WriteLog((T),emRunLog)