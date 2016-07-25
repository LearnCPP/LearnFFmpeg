#pragma once
#include<deque>
#include<mutex>
#include<thread>
#include<memory>
#include "Decoder.h"
#include "D3DRender.h"
#include "Wave.h"

#define __STDC_CONSTANT_MACROS

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};




class CProgram
{
public:
	CProgram();
	~CProgram();
public:
	//初始化
	int Init(int id, AVFormatContext* ic, RECT rt, HWND hWnd);
	//判断音频或者视频是不是本节目的
	inline bool IsProgram(int index) const;
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
	std::shared_ptr<std::thread>	m_thread; //解码线程
	bool							m_bRun;   //线程退出标记
	//audio
	CDecoder						m_audio_dec;//音频解码器
	CWave							m_audio_play; //音频播放
	WAVEFORMATEX					m_wavefmt; //音频播放结构体
	//video
	CDecoder						m_video_dec;//视频解码器
	CD3DRender						m_video_render; //视频渲染
	RECT							m_video_rt;	//视频位置
	HWND							m_hWnd; //窗口句柄
};

