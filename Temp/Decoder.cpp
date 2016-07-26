#include "stdafx.h"
#include "Decoder.h"
#include <iostream>
extern"C"{
#include<libswresample/swresample.h>
#include<libavutil/opt.h>
}


CDecoder::CDecoder()
	: m_avctx(NULL)
	, m_audio_swr_ctx(NULL)
	, m_video_sws_ctx(NULL)
	, m_decoder_reorder_pts(-1)//auto
	, m_start_pts(AV_NOPTS_VALUE)
	/*, m_pkt_serial(-1)*/
	, m_next_pts(AV_NOPTS_VALUE)
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

int CDecoder::Decoder(AVPacket* pkt)
{
	int got_frame = 0;
	AVFrame *frame = av_frame_alloc();
	AVFrame *dst = av_frame_alloc();
	while (pkt->data && pkt->size>0)
	{
		int ret = 0;
		switch (m_avctx->codec_type)
		{
		case AVMEDIA_TYPE_AUDIO:
		{
			ret = avcodec_decode_audio4(m_avctx, frame, &got_frame, pkt);

			if (got_frame) {
				AVRational tb = { 1, frame->sample_rate };
				if (frame->pts != AV_NOPTS_VALUE)
					frame->pts = av_rescale_q(frame->pts, m_avctx->time_base, tb);
				else if (frame->pkt_pts != AV_NOPTS_VALUE)
					frame->pts = av_rescale_q(frame->pkt_pts, av_codec_get_pkt_timebase(m_avctx), tb);
				else if (m_next_pts != AV_NOPTS_VALUE)
					frame->pts = av_rescale_q(m_next_pts, m_next_pts_tb, tb);

				if (frame->pts != AV_NOPTS_VALUE) {
					m_next_pts = frame->pts + frame->nb_samples;
					m_next_pts_tb = tb;
				}
			}
			if (ret < 0 ||(got_frame && !AudioReSample(dst, frame)))
			{
				av_frame_free(&dst);
				std::cout << "decoder audio packet failed!" << std::endl;
			}
		}
		break;
		case  AVMEDIA_TYPE_VIDEO:
		{
			ret = avcodec_decode_video2(m_avctx, frame, &got_frame, pkt);
			if (got_frame)
			{
				if (m_decoder_reorder_pts == -1)
				{
					frame->pts = av_frame_get_best_effort_timestamp(frame);
				}
				else if (m_decoder_reorder_pts)
				{
					frame->pts = frame->pkt_pts;
				}
				else
				{
					frame->pts = frame->pkt_dts;
				}

				av_frame_move_ref(dst, frame);
			}
			else
				av_frame_free(&dst);
		}
		break;
		default:
			std::cout << "AVPacket type isn't audio or video!" << std::endl;
			break;
		}	
		//
		if (ret < 0)
			break;

		pkt->size -= ret;
		pkt->data += ret;

		/* 不足一个帧, 并且packet中还有数据, 继续解码当前音频packet. */
		if (!got_frame && pkt->size > 0)
			continue;

		/* packet中已经没有数据了, 并且不足一个帧, 丢弃这个音频packet. */
		if (pkt->size == 0 && !got_frame)
			break;

		if (dst->linesize[0] != 0 && dst->data[0])
		{
			InnerPutFrame(dst);
			/* packet中数据已经没有数据了, 解码下一个音频packet. */
			if (pkt->size <= 0)
				break;
		}
	}
	
	if (frame)
		av_frame_free(&frame);

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

int CDecoder::Init(AVCodecContext* pCtx, AVStream* st)
{
	m_avctx = pCtx;

	m_start_pts = st->start_time;
	m_start_pts_tb = st->time_base;

	return 0;
}

bool CDecoder::AudioReSample(AVFrame* dst, AVFrame* src)
{
	int nb_sample;
	int dst_buf_size;
	int out_channels;
	int bytes_per_sample = 0;

	*dst = *src;

	out_channels = 2;
	/* 备注: 由于 src->linesize[0] 可能是错误的, 所以计算得到的nb_sample会不正确, 直接使用src->nb_samples即可. */
	nb_sample = src->nb_samples;
	bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	dst_buf_size = nb_sample * bytes_per_sample * out_channels;
	dst->data[0] = (uint8_t*)av_malloc(dst_buf_size);
	if (dst->data[0] == NULL)
		return false;

	avcodec_fill_audio_frame(dst, out_channels, AV_SAMPLE_FMT_S16, dst->data[0], dst_buf_size, 0);

	/* 重采样到AV_SAMPLE_FMT_S16格式. */
	if (m_avctx->sample_fmt != AV_SAMPLE_FMT_S16)
	{
		if (!m_audio_swr_ctx)
		{
			/* create resampler context */
			m_audio_swr_ctx = swr_alloc();
			if (!m_audio_swr_ctx) {
				fprintf(stderr, "Could not allocate resampler context\n");
				//ret = AVERROR(ENOMEM);
			}
			uint64_t in_channel_layout = av_get_default_channel_layout(m_avctx->channels);
			uint64_t out_channel_layout = av_get_default_channel_layout(out_channels);
			/* set options */
			av_opt_set_int(m_audio_swr_ctx, "in_channel_layout", in_channel_layout, 0);
			av_opt_set_int(m_audio_swr_ctx, "in_sample_rate", m_avctx->sample_rate, 0);
			av_opt_set_sample_fmt(m_audio_swr_ctx, "in_sample_fmt", m_avctx->sample_fmt, 0);

			av_opt_set_int(m_audio_swr_ctx, "out_channel_layout", out_channel_layout, 0);
			av_opt_set_int(m_audio_swr_ctx, "out_sample_rate", m_avctx->sample_rate, 0);
			av_opt_set_sample_fmt(m_audio_swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

			/* initialize the resampling context */
			if ((swr_init(m_audio_swr_ctx)) < 0) {
				fprintf(stderr, "Failed to initialize the resampling context\n");
				//goto end;
			}
		}

		if (m_audio_swr_ctx)
		{
			int ret, out_count;
			out_count = dst_buf_size / out_channels / bytes_per_sample;

			ret = swr_convert(m_audio_swr_ctx,
				(uint8_t**)dst->data, out_count,
				(const uint8_t**)src->data, nb_sample);
			if (ret < 0)
			{
				av_free(dst->data[0]);
				swr_free(&m_audio_swr_ctx);
				m_audio_swr_ctx = NULL;
				return false;
			}
			dst->linesize[0] = ret * bytes_per_sample * out_channels;
		}
	}

	return true;
}
