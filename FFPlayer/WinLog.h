#pragma once

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
	CString m_strExePath; //运行程序路径
	CString m_strRunLogName; //运行日志名称
	CString m_strErrorLogName; //错误日志名称
	//const是保证这个单例对象不会被修改，
	//static是保证这个单例对象在程序开始时进入主函数之前就被初始化了，而且是线程安全的。
	static CWinLog* m_sInstance; //静态成员实例变量声明

public:
	static const CWinLog* GetInstance(void)//返回实例句柄
	{
		if (!m_sInstance)
		{
			m_sInstance = new CWinLog;
		}
		return m_sInstance;
	}
	static void ReleaseInstance()//释放实例句柄
	{ 
		if (CWinLog::m_sInstance)
		{
			delete CWinLog::m_sInstance;
			CWinLog::m_sInstance = NULL;
		}
			
	}
	CString GetExePath();
	bool WriteLog(const LPCTSTR& szLog, const int& logtype=emErrorLog) const;
	
private:
	CWinLog(const CString& szLogName = _T("RunLog.log"), const CString& szErrorLogName =_T("ErrorLog.log"));
	CWinLog(CWinLog&);
	bool WriteLogInternal(const LPCTSTR& szLog, const LPCTSTR& szFile,
		const int& line, const LPCTSTR& szLogName)const;
};

#define WriteErrorLog(T) if( CWinLog::GetInstance() != NULL)\
	CWinLog::GetInstance()->WriteLog((T))
						

#define WriteRunLog(T) if( CWinLog::GetInstance() != NULL)\
    CWinLog::GetInstance()->WriteLog((T),emRunLog)