#pragma once

//#include "streamDecoder.h"
//#include "Channel.h"
#include "FFDecoder.h"

class CDecoder
{
public:
	CDecoder(CString url, int port, void* pthis);
	~CDecoder(void);
private:
	CString m_strUrl;
	int m_nPort;
//	CChannel* m_pChannel; 
//	sd_context *m_psd;
	FFContext* m_psd;
	void * m_pParent;
	CString m_strlog; //��־��ʽ��

	DWORD m_dwAudioLastTime; //��¼��Ƶ���һ�λص�ʱ��
	BOOL m_bAudio; //�Ƿ�����audio����������
	DWORD m_dwVideoLastTime; //��¼��Ƶ���һ�λص�ʱ��
	BOOL m_bVideo; //�Ƿ���������video����������
	DWORD m_dwTSLastTime; //��¼ts���һ�λص�ʱ��
	BOOL m_bTs; //�Ƿ����ts����������

	DWORD m_dwStartTime; //����������ʱ��
	BOOL m_bIsRestart; //�Ƿ���������

	int m_nWidth; //��ʼ����Ƶwidth
	int m_nHight; //��ʼ����Ƶheight
public:
	BOOL InitDecoder(int nWidth, int nHeight);
	void  UnInitDecoder(void);
	BOOL RestartDecoder(void);
	BOOL IsNeedRestart(void); 

	void SetLastAudioTime(DWORD dwTime){ m_dwAudioLastTime = dwTime; }
	void SetLastVideoTime(DWORD dwTime){ m_dwVideoLastTime = dwTime; }
	void SetLastTsTime(DWORD dwTime){ m_dwTSLastTime = dwTime; }
	void SetRestartCondition(BOOL bNeed, int nCondition); //nCondition=0 aduio, =1 video, =2 ts
public:
	static int VideoCallback(void * userdata, const FFVideoData *vfd, const FFVideoInfo *vfi);
	static int AudioCallback(void * userdata, const FFAudioData *afd, const FFAudioInfo *afi);
	static int TsCallback(void *userdata, const uint8_t*, int length );
};