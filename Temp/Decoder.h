#pragma once
#include<deque>
#include<mutex>
#include<memory>

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

class CDecoder
{
public:
	CDecoder();
	~CDecoder();
public:
	int Init(AVCodecContext* pCtx);
	int Decoder(AVPacket pkt);
	AVFrame* GetFrame(void);
protected:
	void InnerPutFrame(AVFrame* frame);
	bool AudioReSample(AVFrame* frame);
private:
	AVCodecContext* m_pCodecCtx;
	struct SwrContext *m_audio_swr_ctx; //音频转换指针
	struct SwsContext *m_video_sws_ctx; //视频转换指针

	std::deque<AVFrame*> m_frame; //解码后的帧
	std::shared_ptr<std::mutex> m_mutex;
};

