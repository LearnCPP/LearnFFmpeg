#include "StdAfx.h"
#include "WinLog.h"
#include<io.h>
#include<direct.h>

//外部初始化，静态实例
 CWinLog* CWinLog::m_sInstance = NULL;

CWinLog::CWinLog(const CString& szRunLogName, const CString& szErrorLogName)
	: m_strRunLogName(szRunLogName)
	, m_strErrorLogName(szErrorLogName)
{
	m_strExePath = CWinLog::GetExePath();
}


CWinLog::~CWinLog(void)
{
}

CString CWinLog::GetExePath()
{
	if ( !m_strExePath.IsEmpty() )
	{
		return m_strExePath;
	}

	TCHAR szFullPath[MAX_PATH];
	GetModuleFileName(NULL,szFullPath,MAX_PATH);
	*(_tcsrchr(szFullPath,_T('\\'))+1)=0;

/*#if _DEBUG*/
	memcpy(szFullPath, "d:\\", 4);
/*#endif*/
	return szFullPath;
}

bool CWinLog::WriteLogInternal(const LPCTSTR& szLog, const LPCTSTR& szFile, const int& line,
	const LPCTSTR& szLogName) const
{
	TCHAR szFilePath[MAX_PATH] = { 0 };
	SYSTEMTIME systime;
	GetLocalTime(&systime);

// 	CString strPath = _T("E:\\");
// 	_snprintf_s(szFilePath, MAX_PATH, _T("%sLOG\\%4d-%02d-%02d\\"), strPath, systime.wYear,
// 		systime.wMonth, systime.wDay);
	_snprintf_s(szFilePath, MAX_PATH, _T("%sLOG\\%4d-%02d-%02d\\"), m_strExePath, systime.wYear,
		systime.wMonth, systime.wDay);
	
	if (!PathIsDirectory(szFilePath))
	{//判断日期文件夹是否存在，不存在择创建
		SHCreateDirectoryEx(NULL, szFilePath, NULL);
	}

	_tcsncat_s(szFilePath, szLogName, strlen(szLogName));

	HANDLE hFile = CreateFile(szFilePath,
		GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
		goto FAILED;
	BOOL bRet = FALSE;
	LONG lpDistance = 0;
	bRet = SetFilePointer(hFile, lpDistance, NULL, FILE_END);
	if (bRet == INVALID_SET_FILE_POINTER )
		goto FAILED;
	DWORD dwWriteSize = 0;
	TCHAR szTime[20] = { 0 };
	_snprintf_s(szTime, 20,_T("[%02d-%02d-%02d] "), systime.wHour, systime.wMinute, systime.wSecond);
	bRet = WriteFile(hFile, szTime, strlen(szTime), &dwWriteSize, NULL);
	if (!bRet || dwWriteSize < 0)
		goto FAILED;
	if ( szFile!=NULL || line!=-1 )
	{
		CString str;
		str.Format(_T("[filename:%s line:%d]"), szFile, line);
		bRet = WriteFile(hFile, str, str.GetLength(), &dwWriteSize, NULL);
		if (!bRet || dwWriteSize < 0)
			goto FAILED;
	}
	bRet = WriteFile(hFile, szLog, strlen(szLog), &dwWriteSize, NULL);
	if (!bRet || dwWriteSize < 0)
		goto FAILED;
	bRet = WriteFile(hFile, _T("\r\n"), 2, &dwWriteSize, NULL);
	if (!bRet || dwWriteSize < 0)
		goto FAILED;

	CloseHandle(hFile);
	return true;

FAILED:
	CloseHandle(hFile);
	return false;
}

bool CWinLog::WriteLog(const LPCTSTR& szLog, const int& logtype/*=emErrorLog*/) const
{
	bool bRet = false;

	switch (logtype)
	{
	case emErrorLog:
		bRet = WriteLogInternal(szLog, NULL,-1,/*__FILE__, __LINE__,*/ m_strErrorLogName);
		break;
	case emRunLog:
		bRet = WriteLogInternal(szLog, NULL, -1, m_strRunLogName);
		break;
	}

	return bRet;
}


