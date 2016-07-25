#include "stdafx.h"
#include "Program.h"
#include <iostream>

CProgram::CProgram()
	: m_id(-1)
	, m_fmt_ctx(NULL)
	, m_videoIndex(-1)
	, m_audioIndex(-1)
	, m_pVideoCodecCtx(NULL)
	, m_pVideoCodec(NULL)
	, m_pAudioCodecCtx(NULL)
	, m_pAudioCodec(NULL)
	, m_bRun(false)
	, m_hWnd(NULL)
{
	memset(&m_wavefmt, 0, sizeof(m_wavefmt));
}


CProgram::~CProgram()
{
	StopDecoder();
}

void CProgram::SetStreamIndex(int index, AVMediaType type)
{
	if (AVMEDIA_TYPE_VIDEO == type)
		m_videoIndex = index;
	else if (AVMEDIA_TYPE_AUDIO == type)
		m_audioIndex = index;
	else
		;
}

int CProgram::GetStreamIndex(AVMediaType type) const
{
	if (AVMEDIA_TYPE_VIDEO == type)
		return m_videoIndex;
	else if (AVMEDIA_TYPE_AUDIO == type)
		return m_audioIndex;
	else
		return -1;
}

bool CProgram::FindOpenCodec(void)
{
	bool bAResult = false;
	bool bVResult = false;

	if (bAResult = InnerOpenAudioCodec())
		std::cout << "Open Audio Codec Success." << std::endl;
	else
		std::cout << "Open Audio Codec Failed." << std::endl;

	if (bVResult = InnerOpenVideoCodec())
		std::cout << "Open Video Codec Success." << std::endl;
	else
		std::cout << "Open Video Codec Failed." << std::endl;

	return (bAResult && bVResult);
}

bool CProgram::IsProgram(int index) const
{
	if (index == m_videoIndex || index == m_audioIndex)
		return true;
	else
		return false;
}

bool CProgram::InnerOpenVideoCodec(void)
{
	int nRet = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_AUDIO, m_videoIndex, -1, NULL, 0);
	if (nRet == AVERROR_STREAM_NOT_FOUND)
	{
		std::cout << "have no find stream " << m_videoIndex << std::endl;
		return false;
	}
	m_pVideoCodecCtx = m_fmt_ctx->streams[m_videoIndex]->codec; //audio解码器上下文
	if (!m_pVideoCodecCtx)
	{
		std::cout << "audio AVCodecContext is null" << std::endl;
		return false;
	}
	m_pVideoCodec = avcodec_find_decoder(m_pVideoCodecCtx->codec_id);//查找解码器
	if (!m_pVideoCodec)
	{
		std::cout << "can't find video decoder. codec_id = " << m_pVideoCodecCtx->codec_id << std::endl;
		return false;
	}
	nRet = avcodec_open2(m_pVideoCodecCtx, m_pVideoCodec, NULL); //打开解码器
	if (nRet < 0)
	{
		std::cout << "can't open video decoder. codec name = " << m_pVideoCodec->name << std::endl;
		return false;
	}

	m_video_dec.Init(m_pVideoCodecCtx);

	return true;
}

bool CProgram::InnerOpenAudioCodec(void)
{
	int nRet = av_find_best_stream(m_fmt_ctx, AVMEDIA_TYPE_AUDIO, m_audioIndex, -1, NULL, 0);
	if (nRet == AVERROR_STREAM_NOT_FOUND)
	{
		std::cout << "have no find stream " << m_audioIndex << std::endl;
		return false;
	}
	m_pAudioCodecCtx = m_fmt_ctx->streams[m_audioIndex]->codec; //audio解码器上下文
	if (!m_pAudioCodecCtx)
	{
		std::cout << "audio AVCodecContext is null" << std::endl;
		return false;
	}
	m_pAudioCodec = avcodec_find_decoder(m_pAudioCodecCtx->codec_id);//查找解码器
	if (!m_pAudioCodec)
	{
		std::cout << "can't find audio decoder. codec_id = " << m_pAudioCodecCtx->codec_id << std::endl;
		return false;
	}
	nRet = avcodec_open2(m_pAudioCodecCtx, m_pAudioCodec, NULL); //打开解码器
	if (nRet < 0)
	{
		std::cout << "can't open audio decoder. codec name = " << m_pAudioCodec->name << std::endl;
		return false;
	}

	m_audio_dec.Init(m_pAudioCodecCtx);

	return true;
}

int CProgram::SetPacket(AVPacket* pkt)
{
	if (!m_bRun)
		return 0;

	if (!pkt || av_dup_packet(pkt) < 0)
		return -1;

	std::lock_guard<std::mutex> lock(*m_pktMutex);
	m_pkt.push_back(*pkt);

	return 0;
}

bool CProgram::StartDecoder(void)
{
	m_bRun = true;
	m_pktMutex = std::make_shared<std::mutex>();
	m_thread = std::make_shared<std::thread>(std::bind(&CProgram::DecoderThread, this));

	return true;
}

int CProgram::DecoderThread()
{
	while (m_bRun)
	{
		if (m_pkt.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
			continue;
		}

		AVPacket pkt = GetPacket();
		if (pkt.stream_index == m_audioIndex)
		{
			m_audio_dec.Decoder(pkt);
		}
		else if (pkt.stream_index == m_videoIndex)
		{
			m_video_dec.Decoder(pkt);
		}
		else
			;
	}

	return 0;
}

void CProgram::StopDecoder(void)
{
	if (m_bRun)
	{
		m_bRun = false;
		m_thread->join();
		m_thread.reset();
	}

	m_pktMutex.reset();
}

AVPacket&& CProgram::GetPacket(void)
{
	std::lock_guard<std::mutex> lock(*m_pktMutex);
	AVPacket pkt = m_pkt.front();
	m_pkt.pop_front();

	return std::move(pkt);
}

int CProgram::Init(int id, AVFormatContext* ic, RECT rt, HWND hWnd)
{
	m_id = id;
	m_fmt_ctx = ic;
	m_video_rt = rt;
	m_hWnd = hWnd;

	m_video_render.init_render(m_hWnd, rt.right - rt.left, rt.bottom - rt.top, 1);

	return 0;
}

int CProgram::DisplayVideoThread()
{
	while (m_bRun)
	{
		AVFrame* frame = m_video_dec.GetFrame();
		if (!frame)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			continue;
		}

		m_video_render.render_one_frame(frame, m_video_rt, false);
	}

	return 0;
}

int CProgram::DisplayAudioThread()
{
	while (m_bRun)
	{
		AVFrame* frame = m_audio_dec.GetFrame();
		if (!frame)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			continue;
		}
		if (frame->channels != m_wavefmt.nChannels 
			|| frame->sample_rate != m_wavefmt.nSamplesPerSec)
		{
			m_wavefmt.nChannels = frame->channels;
			m_wavefmt.nSamplesPerSec = frame->sample_rate;
			m_wavefmt.cbSize = sizeof(WAVEFORMATEX);
			m_wavefmt.wBitsPerSample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
			m_wavefmt.nBlockAlign = m_wavefmt.wBitsPerSample / 8 * m_wavefmt.nChannels;
			m_wavefmt.nAvgBytesPerSec = m_wavefmt.nBlockAlign * m_wavefmt.nSamplesPerSec; //每秒播放字节数
		
			m_audio_play.Start(&m_wavefmt);
		}

		//m_audio_play.Play(frame->extended_data,frame->nb_samples)
	}

	return 0;
}
