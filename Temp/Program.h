#pragma once
#include<deque>
#include<mutex>
#include<thread>
#include<memory>

#define __STDC_CONSTANT_MACROS

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};




class CProgram
{
public:
	CProgram(int id, AVFormatContext* ic);
	~CProgram();
public:
	//判断音频或者视频是不是本节目的
	inline bool IsProgram(int index) const;
	//设置此节目视频，音频索引
	inline void SetStreamIndex(int index, AVMediaType type);
	//获取此节目视频，音频索引
	inline int  GetStreamIndex(AVMediaType type)const;
	//打开解码器
	bool FindOpenCodec(void);
	//设置AVPacket包
	int SetPacket(AVPacket* pkt);
	//启动解码线程
	bool StartDecoder(void);
	//停止解码线程
	void StopDecoder(void);

	//friend CProgram* GetThis(){ return this; }
private:
	bool InnerOpenVideoCodec(void);
	bool InnerOpenAudioCodec(void);
	int  DecoderThread(); //解码线程
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
};

