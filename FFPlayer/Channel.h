#pragma once
//#include"CircularQueue.h"
#include "Decoder.h"
#include "D3DRender.h"


// struct _tagVideoHead 
// {
// 	int width;
// 	int height;
// 	int linesizeY;
// 	int linesizeU;
// 	int linesizeV;
// };

enum VideoDialogState{
	emNormal, //����
	emFullScreen,//ȫ��
	emOtherFullScreen //��������ȫ��
};

class CChannel
{
public:
	CChannel(HWND hWnd/*, int id*/);
	~CChannel(void);
private:
	HWND m_hWnd; //�����ھ��
	int m_nID; //��ĿID
	CRect m_rtVideo; //��Ƶ����
	CRect m_rtAudio; //��������

	CD3DRender* m_pD3DRender;

//	CCircularQueue * m_pQueue; //ѭ�����У��洢����
	CDecoder* m_pDecoder; //����������
	BOOL m_bPlaySound; //�Ƿ񲥷���Ƶ
	CString m_strlog; //��ʽ����־���

	BOOL m_bIsFullSerccn; //�Ƿ���ȫ��
	BOOL m_bOldPlaySound; //��¼ȫ��֮ǰ��Ƶ����״̬
	CRect m_rectFullSreen;
	///////////////////////////////��������///////////////////////////////////////////
public:
	CRITICAL_SECTION m_Critical;
	HANDLE m_hThreadPop;		// ��ͼ�����߳�
	BOOL m_bChannelExited; //�˳���־

	int m_cColorBar;			// ������Ŀ
	int m_nColorBarPixcel;		// �������ظ߶�
	int m_nSeparatorPixcel;		// ��϶���ظ߶�
	int m_nColorBarWidth;		// �������ؿ��
	int m_nSeparatorWidth;		// ��϶���ؿ��
	int m_nVal_l;				// ��ǰ����Ƶֵ
	int m_nVal_r;				// ��ǰ����Ƶֵ
	int m_nPop_l;				// ��ǰ�ĵ���ֵ
	int m_nPop_r;				// ��ǰ�ĵ���ֵ
	int m_dVal_l;				// �����������ٶ�
	int m_dVal_r;				// �����������ٶ�
	int m_cVal_l;				// ���ٶȼ�����
	int m_cVal_r;				// ���ٶȼ�����

	BOOL m_bEnable;				// �Ƿ������ʾ��Ƶ��ͼ
	DWORD	m_dwTime;			// ǰ��ʱ��
	DWORD	m_old_max;			// ǰ��10���������Ƶ����ֵ
	DWORD	m_new_max;			// Ŀǰ�����Ƶ����ֵ

private:
	void DrawVal(HDC hDC, RECT rect);						// ������Ƶ��ͼ
	void inerDrawValue(HDC hDC, int cx, int cy);
	void DrawOneTrack( HDC hDC, int nVal, int &nPop, int left, int top, int right, int bottom );	// ����һ������������
	void DrawAudioVal( HWND hWnd );							// ��ʼ������Ƶ��ͼ
	void put_AudioVal( int newVal_l, int newVal_r ,DWORD realVal);		// ������Ƶֵ(0~255)
	static DWORD WINAPI jmp_ThreadPop( LPVOID pParam );
	void ThreadPop(void);						// �����߳�
	void Destory(); //������Դ��ֹͣ�߳�
	

public:
	void putVideoOrAudioRect(int x, int y, int w, int h, BOOL bVideo=TRUE);//������Ƶ���������꣬bVideo=true����Ƶ����֮Ϊ��Ƶ
	CRect GetVideoOrAudioRect(BOOL bVideo=TRUE) const; //������Ƶ����Ƶ��������
	BOOL InitAV(CString url, int port); //��ʼ��audio��video����
	void AudioCallback2( DWORD l, DWORD r ); //��������������ǰ��Ƶֵ
//	void SaveVideoData(const FFVideoData* pVFD,const FFVideoInfo *pVFI);//���������YUV��ƵͼƬ
//	BOOL DrawVideo(void); //��ʾ��Ƶ����
	BOOL DrawVideo(const FFVideoData* pVFD, const FFVideoInfo *pVFI);
	void SetAudioPlay(BOOL bPlay){ m_bPlaySound = bPlay; }; //���õ�ǰƵ���Ƿ񲥷���Ƶֵ
	BOOL GetAudioPlay(void)const{ return m_bPlaySound; }; //��ȡ��ǰƵ���Ƿ񲥷���Ƶֵ
	BOOL IsInChannelRect(CPoint &point) const;//

	void SetDecoderLastTime(DWORD dwTime, int type); //type=0 audio, =1 video, =2 ts
	void SetDecoderNeedRestart(BOOL bNeed, int nCondition);
	void DetecteDecoderRestart(void);//���������Ƿ���Ҫ����

	void SetFullScreen(BOOL bFull);
	BOOL GetFullScreen(void) const{ return m_bIsFullSerccn; }
	BOOL GetAudioPlayBeforeFull(void)const{ return m_bOldPlaySound; }
};