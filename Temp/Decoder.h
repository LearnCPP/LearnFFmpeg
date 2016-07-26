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
	AVCodecContext* m_avctx; //������������
	struct SwrContext *m_audio_swr_ctx; //��Ƶת��ָ��
	struct SwsContext *m_video_sws_ctx; //��Ƶת��ָ��

	std::deque<AVFrame*> m_frame; //������֡
	std::shared_ptr<std::mutex> m_mutex;

	int m_decoder_reorder_pts; //��Ƶ�Ƿ��ý�������������PTS 0=off,1=on,-1=auto
	int64_t m_start_pts; //��ʼpts
	AVRational m_start_pts_tb; //��ʼavrational
	int64_t m_next_pts; //��һ֡pts
	AVRational m_next_pts_tb; //��һ֡avrational
};

