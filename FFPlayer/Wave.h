#pragma once
#include <windows.h>
#include <mmsystem.h>

#define	BLOCK_COUNT	40
#define	BLOCK_SIZE	(20*1024)

typedef struct _WAVE_FILE_HEAD
{
	char	riff[4];			// 'RIFF'
	DWORD	dwFileSize;			// 文件字节长度 - 8
	char	fmt[8];				// 'WAVEfmt '
	DWORD	dwCom;				// 0x10(PCM)
	WORD	wFmtTag;			// 0x01
	WORD	wChannels;			// 声道数
	DWORD	dwSamplesPerSec;	// 采样率
	DWORD	dwAvgBytesPerSec;	// 每秒播放字节数
	WORD	wBlockAlign;		// 样本字节数
	WORD	wBitsPerSample;		// 采样精度
	char	data[4];			// 'data'
	DWORD	dwDataLen;			// 数据长=文件长-44
} WAVE_FILE_HEAD, *PWAVE_FILE_HEAD;

class CWave
{
public:
	CWave(void);
	~CWave(void);
private:
	CRITICAL_SECTION m_Critical;

	HWAVEOUT		m_hWaveOut;
	WAVEHDR			m_outWaveHdr;
	WAVEFORMATEX	m_waveout;
	DWORD			m_idThreadWave;	// 音频处理线程ID
	HANDLE			m_hThreadWave;	// 音频处理线程句柄

	DWORD			m_dwOutVal;		// 音量大小

	WAVEHDR m_hdr[BLOCK_COUNT];
	int		m_flag[BLOCK_COUNT];	// m_hdr 的使用标记:0 = 自由，1=播放，-1=缓冲

public:
	bool Start(WAVEFORMATEX *pwaveformat);//int bps);		// 开始录放音
	bool Close(void);								// 停止录放音
	bool WaveMsg( UINT message, WAVEHDR * pWH );	// 线程消息处理
	void Play(char * pBuf, int nLen);				// 播放声音
	void WomDone(PWAVEHDR pWaveHdr);				// 当缓冲区播放完成的时候
	void OutSetVal(DWORD dwVal)						// 设置音量
	{
		::waveOutSetVolume( m_hWaveOut, dwVal );
		::waveOutGetVolume( m_hWaveOut, &m_dwOutVal );
	}
	DWORD OutGetVal(void)							// 取得音量
	{
		return m_dwOutVal;
	}
};
