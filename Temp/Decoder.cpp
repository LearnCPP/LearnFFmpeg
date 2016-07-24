#include "stdafx.h"
#include "Decoder.h"
#include <iostream>


CDecoder::CDecoder(AVCodecContext* pCtx)
	: m_pCodecCtx(pCtx)
{
	m_mutex = std::make_shared<std::mutex>();
}


CDecoder::~CDecoder()
{
	{
		std::lock_guard<std::mutex> lock(*m_mutex);

	}
}

void CDecoder::InnerPutFrame(AVFrame&& frame)
{
	std::lock_guard<std::mutex> lock(*m_mutex);
	m_frame.push_back(frame);
}

int CDecoder::Decoder(AVPacket pkt)
{
	int got_frame = 0;
	while (!pkt.data && pkt.size>0)
	{
		AVFrame avframe;
		int ret = 0;
		if (m_pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
			ret = avcodec_decode_audio4(m_pCodecCtx, &avframe, &got_frame, &pkt);
		else if (m_pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
			ret = avcodec_decode_video2(m_pCodecCtx, &avframe, &got_frame, &pkt);
		else
		{
			std::cout << "AVPacket type isn't audio or video!" << std::endl;
			return -1;
		}
		if (ret < 0)
		{
			std::cout << "packet decoding on frame error!" << std::endl;
			return ret;
		}
		pkt.size -= ret;
		pkt.data += ret;

		/* 不足一个帧, 并且packet中还有数据, 继续解码当前音频packet. */
		if (!got_frame && pkt.size > 0)
			continue;

		/* packet中已经没有数据了, 并且不足一个帧, 丢弃这个音频packet. */
		if (pkt.size == 0 && !got_frame)
			break;

		if (avframe.linesize[0] != 0 && avframe.data[0])
		{
			InnerPutFrame(std::move(avframe));
			/* packet中数据已经没有数据了, 解码下一个音频packet. */
			if (pkt.size <= 0)
				break;
		}
	}
	
	return got_frame;
}

AVFrame CDecoder::GetFrame(void)
{
	std::lock_guard<std::mutex> lock(*m_mutex);

	AVFrame frame = m_frame.front();
	m_frame.pop_front();

	return std::move(frame);
}
