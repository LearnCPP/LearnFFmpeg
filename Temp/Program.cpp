#include "stdafx.h"
#include "Program.h"
#include <iostream>

CProgram::CProgram(int id, AVFormatContext* ic)
	: m_id(id)
	, m_fmt_ctx(ic)
	, m_videoIndex(-1)
	, m_audioIndex(-1)
	, m_pVideoCodecCtx(NULL)
	, m_pVideoCodec(NULL)
	, m_pAudioCodecCtx(NULL)
	, m_pAudioCodec(NULL)
	, m_bRun(false)
{
}


CProgram::~CProgram()
{
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
