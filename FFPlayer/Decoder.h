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
	CString m_strlog; //日志格式化

	DWORD m_dwAudioLastTime; //记录音频最后一次回调时间
	BOOL m_bAudio; //是否启动audio重启解码器
	DWORD m_dwVideoLastTime; //记录视频最后一次回调时间
	BOOL m_bVideo; //是否启动根据video重启解码器
	DWORD m_dwTSLastTime; //记录ts最后一次回调时间
	BOOL m_bTs; //是否根据ts重启解码器

	DWORD m_dwStartTime; //解码器启动时间
	BOOL m_bIsRestart; //是否正在重启

	int m_nWidth; //初始化视频width
	int m_nHight; //初始化视频height
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