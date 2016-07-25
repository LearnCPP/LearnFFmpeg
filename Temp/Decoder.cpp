#include "stdafx.h"
#include "Decoder.h"
#include <iostream>
extern"C"{
#include<libswresample/swresample.h>
}


CDecoder::CDecoder()
	: m_pCodecCtx(NULL)
	, m_audio_swr_ctx(NULL)
	, m_video_sws_ctx(NULL)
{
	m_mutex = std::make_shared<std::mutex>();
}


CDecoder::~CDecoder()
{
	{
		std::lock_guard<std::mutex> lock(*m_mutex);
		auto it = m_frame.begin();
		if (it != m_frame.end())
		{
			av_frame_free(&(*it));
			++it;
		}
		m_frame.clear();
	}
}

void CDecoder::InnerPutFrame(AVFrame* frame)
{
	std::lock_guard<std::mutex> lock(*m_mutex);
	m_frame.push_back(frame);
}

int CDecoder::Decoder(AVPacket pkt)
{
	int got_frame = 0;
	while (!pkt.data && pkt.size>0)
	{
		AVFrame *avframe = av_frame_alloc();
		int ret = 0;
		if (m_pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			ret = avcodec_decode_audio4(m_pCodecCtx, avframe, &got_frame, &pkt);
		}	
		else if (m_pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
			ret = avcodec_decode_video2(m_pCodecCtx, avframe, &got_frame, &pkt);
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

		if (avframe->linesize[0] != 0 && avframe->data[0])
		{
			InnerPutFrame(std::move(avframe));
			/* packet中数据已经没有数据了, 解码下一个音频packet. */
			if (pkt.size <= 0)
				break;
		}
	}
	
	return got_frame;
}

AVFrame* CDecoder::GetFrame(void)
{
	std::lock_guard<std::mutex> lock(*m_mutex);

	if (m_frame.empty())
		return NULL;

	AVFrame* frame = m_frame.front();
	m_frame.pop_front();

	return frame;
}

int CDecoder::Init(AVCodecContext* pCtx)
{
	m_pCodecCtx = pCtx;

	return 0;
}

bool CDecoder::AudioReSample(AVFrame* frame)
{
	int nb_sample;
	int dst_buf_size;
	int out_channels;
	int bytes_per_sample = 0;

	out_channels = 2;
	bytes_per_sample = av_get_bytes_per_sample(m_pCodecCtx->sample_fmt);
	/* 备注: 由于 src->linesize[0] 可能是错误的, 所以计算得到的nb_sample会不正确, 直接使用src->nb_samples即可. */
	bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	dst_buf_size = nb_sample * bytes_per_sample * out_channels;
	dst->data[0] = (uint8_t*)av_malloc(dst_buf_size);
	assert(dst->data[0]);
	avcodec_fill_audio_frame(dst, out_channels, AV_SAMPLE_FMT_S16, dst->data[0], dst_buf_size, 0);

	/* 重采样到AV_SAMPLE_FMT_S16格式. */
	if (m_pCodecCtx->sample_fmt != AV_SAMPLE_FMT_S16)
	{
		if (!m_audio_swr_ctx)
		{
			/* create resampler context */
			vs->m_audio_swr_ctx = swr_alloc();
			if (!vs->m_audio_swr_ctx) {
				fprintf(stderr, "Could not allocate resampler context\n");
				//ret = AVERROR(ENOMEM);
			}
			uint64_t in_channel_layout = av_get_default_channel_layout(vs->m_audio_ctx->channels);
			uint64_t out_channel_layout = av_get_default_channel_layout(out_channels);
			/* set options */
			av_opt_set_int(vs->m_audio_swr_ctx, "in_channel_layout", in_channel_layout, 0);
			av_opt_set_int(vs->m_audio_swr_ctx, "in_sample_rate", vs->m_audio_ctx->sample_rate, 0);
			av_opt_set_sample_fmt(vs->m_audio_swr_ctx, "in_sample_fmt", vs->m_audio_ctx->sample_fmt, 0);

			av_opt_set_int(vs->m_audio_swr_ctx, "out_channel_layout", out_channel_layout, 0);
			av_opt_set_int(vs->m_audio_swr_ctx, "out_sample_rate", vs->m_audio_ctx->sample_rate, 0);
			av_opt_set_sample_fmt(vs->m_audio_swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

			/* initialize the resampling context */
			if ((swr_init(vs->m_audio_swr_ctx)) < 0) {
				fprintf(stderr, "Failed to initialize the resampling context\n");
				//goto end;
			}
		}

		if (m_audio_swr_ctx)
		{
			int ret, out_count;
			out_count = dst_buf_size / out_channels / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
			//ret = swr_convert(vs->m_audio_swr_ctx, (uint8_t**)dst->data[0], out_count, (const uint8_t**)src->data[0], nb_sample);
			ret = swr_convert(vs->m_audio_swr_ctx,
				(uint8_t**)dst->data, out_count,
				(const uint8_t**)src->data, nb_sample);
			if (ret < 0)
				assert(0);
			dst->linesize[0] = ret * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * out_channels;
			//			src->linesize[0] = dst->linesize[0] = ret * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * out_channels;
			//			memcpy(src->data[0], dst->data[0], src->linesize[0]);
		}
	}
}
