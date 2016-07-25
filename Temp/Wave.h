#pragma once
#include <windows.h>
#include <mmsystem.h>

#define	BLOCK_COUNT	40
#define	BLOCK_SIZE	(20*1024)

typedef struct _WAVE_FILE_HEAD
{
	char	riff[4];			// 'RIFF'
	DWORD	dwFileSize;			// �ļ��ֽڳ��� - 8
	char	fmt[8];				// 'WAVEfmt '
	DWORD	dwCom;				// 0x10(PCM)
	WORD	wFmtTag;			// 0x01
	WORD	wChannels;			// ������
	DWORD	dwSamplesPerSec;	// ������
	DWORD	dwAvgBytesPerSec;	// ÿ�벥���ֽ���
	WORD	wBlockAlign;		// �����ֽ���
	WORD	wBitsPerSample;		// ��������
	char	data[4];			// 'data'
	DWORD	dwDataLen;			// ���ݳ�=�ļ���-44
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
	DWORD			m_idThreadWave;	// ��Ƶ�����߳�ID
	HANDLE			m_hThreadWave;	// ��Ƶ�����߳̾��

	DWORD			m_dwOutVal;		// ������С

	WAVEHDR m_hdr[BLOCK_COUNT];
	int		m_flag[BLOCK_COUNT];	// m_hdr ��ʹ�ñ��:0 = ���ɣ�1=���ţ�-1=����

public:
	bool Start(WAVEFORMATEX *pwaveformat);//int bps);		// ��ʼ¼����
	bool Close(void);								// ֹͣ¼����
	bool WaveMsg( UINT message, WAVEHDR * pWH );	// �߳���Ϣ����
	void Play(char * pBuf, int nLen);				// ��������
	void WomDone(PWAVEHDR pWaveHdr);				// ��������������ɵ�ʱ��
	void OutSetVal(DWORD dwVal)						// ��������
	{
		::waveOutSetVolume( m_hWaveOut, dwVal );
		::waveOutGetVolume( m_hWaveOut, &m_dwOutVal );
	}
	DWORD OutGetVal(void)							// ȡ������
	{
		return m_dwOutVal;
	}
};
