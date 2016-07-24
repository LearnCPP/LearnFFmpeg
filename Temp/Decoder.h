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
	CDecoder(AVCodecContext* pCtx);
	~CDecoder();
public:
	int Decoder(AVPacket pkt);
	AVFrame GetFrame(void);
protected:
	void InnerPutFrame(AVFrame&& frame);
private:
	AVCodecContext* m_pCodecCtx;

	std::deque<AVFrame> m_frame; //½âÂëºóµÄÖ¡
	std::shared_ptr<std::mutex> m_mutex;
};

