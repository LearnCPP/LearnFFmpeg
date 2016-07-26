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
	int Init(AVCodecContext* pCtx, AVStream* st);
	int Decoder(AVPacket* pkt);
	AVFrame* GetFrame(void);
protected:
	void InnerPutFrame(AVFrame* frame);
	bool AudioReSample(AVFrame* dst, AVFrame* src);
private:
	AVCodecContext* m_avctx; //解码器上下文
	struct SwrContext *m_audio_swr_ctx; //音频转换指针
	struct SwsContext *m_video_sws_ctx; //视频转换指针

	std::deque<AVFrame*> m_frame; //解码后的帧
	std::shared_ptr<std::mutex> m_mutex;

	int m_decoder_reorder_pts; //视频是否让解码器重新排序PTS 0=off,1=on,-1=auto
	int64_t m_start_pts; //起始pts
	AVRational m_start_pts_tb; //起始avrational
	int64_t m_next_pts; //下一帧pts
	AVRational m_next_pts_tb; //下一帧avrational
};

