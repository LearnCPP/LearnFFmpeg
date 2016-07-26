#pragma once
#include<deque>
#include<mutex>
#include<thread>
#include<memory>
#include "Decoder.h"
#include "D3DRender.h"
#include "Wave.h"
#include <condition_variable>

#define __STDC_CONSTANT_MACROS

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};


const double  AV_SYNC_THRESHOLD_MAX = 0.1; //音视频同步门限，大于则丢帧处理

class CProgram
{
public:
	CProgram();
	~CProgram();
public:
	//初始化
	int Init(int id, AVFormatContext* ic, RECT rt, HWND hWnd,
		std::shared_ptr<std::condition_variable> cond);
	//判断音频或者视频是不是本节目的
	bool IsProgram(int index) const;
	//设置此节目视频，音频索引
	void SetStreamIndex(int index, AVMediaType type);
	//获取此节目视频，音频索引
	inline int  GetStreamIndex(AVMediaType type)const;
	//打开解码器
	bool FindOpenCodec(void);
	//设置AVPacket包
	int SetPacket(AVPacket* pkt);
	//获取AVPacketbao
	AVPacket&& GetPacket(void);
	//启动解码线程
	bool StartDecoder(void);
	//停止解码线程
	void StopDecoder(void);

	//friend CProgram* GetThis(){ return this; }
private:
	bool InnerOpenVideoCodec(void);
	bool InnerOpenAudioCodec(void);
	int  DecoderThread(); //解码线程
	int  DisplayVideoThread(); //显示视频线程
	int  DisplayAudioThread(); //显示视频线程
private:
	int					m_id;
	int					m_videoIndex;
	int					m_audioIndex;
	AVCodecContext*		m_pAudioCodecCtx; //解码器上下文
	AVCodec*			m_pAudioCodec; //解码器
	AVCodecContext*		m_pVideoCodecCtx;
	AVCodec*			m_pVideoCodec;
	AVFormatContext*	m_fmt_ctx;

	std::deque<AVPacket>			m_pkt;
	std::shared_ptr<std::mutex>		m_pktMutex; //队列锁
	std::shared_ptr<std::mutex>		m_videoPtsMtx; //视频pts锁
	std::shared_ptr<std::mutex>		m_audioPtsMtx; //音频pts锁
	std::shared_ptr<std::thread>	m_thread; //解码线程
	std::shared_ptr<std::thread>	m_videoThread; //video display thread
	std::shared_ptr<std::thread>	m_audioThread; //audio display thread
	bool							m_bRun;   //线程退出标记
	//audio
	CDecoder						m_audio_dec;//音频解码器
	CWave							m_audio_play; //音频播放
	WAVEFORMATEX					m_wavefmt; //音频播放结构体
	int64_t							m_audio_pts; //音频当前时间戳
	//video
	CDecoder						m_video_dec;//视频解码器
	CD3DRender						m_video_render; //视频渲染
	RECT							m_video_rt;	//视频位置
	HWND							m_hWnd; //窗口句柄
	int64_t							m_video_pts; //视频当前时间戳

	std::weak_ptr<std::condition_variable> m_empty_cond; //
	//音视频简单同步，以音频为主
	
};

