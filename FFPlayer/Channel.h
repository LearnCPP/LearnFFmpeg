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
	emNormal, //正常
	emFullScreen,//全屏
	emOtherFullScreen //其他窗口全屏
};

class CChannel
{
public:
	CChannel(HWND hWnd/*, int id*/);
	~CChannel(void);
private:
	HWND m_hWnd; //主窗口句柄
	int m_nID; //节目ID
	CRect m_rtVideo; //视频坐标
	CRect m_rtAudio; //音柱坐标

	CD3DRender* m_pD3DRender;

//	CCircularQueue * m_pQueue; //循环队列，存储数据
	CDecoder* m_pDecoder; //解码器对象
	BOOL m_bPlaySound; //是否播放音频
	CString m_strlog; //格式化日志输出

	BOOL m_bIsFullSerccn; //是否是全屏
	BOOL m_bOldPlaySound; //记录全屏之前音频播放状态
	CRect m_rectFullSreen;
	///////////////////////////////音柱绘制///////////////////////////////////////////
public:
	CRITICAL_SECTION m_Critical;
	HANDLE m_hThreadPop;		// 柱图弹跳线程
	BOOL m_bChannelExited; //退出标志

	int m_cColorBar;			// 彩条数目
	int m_nColorBarPixcel;		// 彩条像素高度
	int m_nSeparatorPixcel;		// 间隙像素高度
	int m_nColorBarWidth;		// 彩条像素宽度
	int m_nSeparatorWidth;		// 间隙像素宽度
	int m_nVal_l;				// 当前的音频值
	int m_nVal_r;				// 当前的音频值
	int m_nPop_l;				// 当前的弹跳值
	int m_nPop_r;				// 当前的弹跳值
	int m_dVal_l;				// 弹跳的下落速度
	int m_dVal_r;				// 弹跳的下落速度
	int m_cVal_l;				// 加速度计数器
	int m_cVal_r;				// 加速度计数器

	BOOL m_bEnable;				// 是否可以显示音频柱图
	DWORD	m_dwTime;			// 前次时间
	DWORD	m_old_max;			// 前次10秒内最大音频幅度值
	DWORD	m_new_max;			// 目前最大音频幅度值

private:
	void DrawVal(HDC hDC, RECT rect);						// 绘制音频柱图
	void inerDrawValue(HDC hDC, int cx, int cy);
	void DrawOneTrack( HDC hDC, int nVal, int &nPop, int left, int top, int right, int bottom );	// 绘制一个声道的数据
	void DrawAudioVal( HWND hWnd );							// 开始绘制音频柱图
	void put_AudioVal( int newVal_l, int newVal_r ,DWORD realVal);		// 设置音频值(0~255)
	static DWORD WINAPI jmp_ThreadPop( LPVOID pParam );
	void ThreadPop(void);						// 泡泡线程
	void Destory(); //销毁资源，停止线程
	

public:
	void putVideoOrAudioRect(int x, int y, int w, int h, BOOL bVideo=TRUE);//设置视频和音柱坐标，bVideo=true是视频，反之为音频
	CRect GetVideoOrAudioRect(BOOL bVideo=TRUE) const; //返回视频或音频矩形区域
	BOOL InitAV(CString url, int port); //初始化audio和video设置
	void AudioCallback2( DWORD l, DWORD r ); //设置左右音柱当前音频值
//	void SaveVideoData(const FFVideoData* pVFD,const FFVideoInfo *pVFI);//保存解码后的YUV视频图片
//	BOOL DrawVideo(void); //显示视频数据
	BOOL DrawVideo(const FFVideoData* pVFD, const FFVideoInfo *pVFI);
	void SetAudioPlay(BOOL bPlay){ m_bPlaySound = bPlay; }; //设置当前频道是否播放音频值
	BOOL GetAudioPlay(void)const{ return m_bPlaySound; }; //获取当前频道是否播放音频值
	BOOL IsInChannelRect(CPoint &point) const;//

	void SetDecoderLastTime(DWORD dwTime, int type); //type=0 audio, =1 video, =2 ts
	void SetDecoderNeedRestart(BOOL bNeed, int nCondition);
	void DetecteDecoderRestart(void);//检测解码器是否需要重启

	void SetFullScreen(BOOL bFull);
	BOOL GetFullScreen(void) const{ return m_bIsFullSerccn; }
	BOOL GetAudioPlayBeforeFull(void)const{ return m_bOldPlaySound; }
};