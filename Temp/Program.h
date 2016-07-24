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
	//�ж���Ƶ������Ƶ�ǲ��Ǳ���Ŀ��
	inline bool IsProgram(int index) const;
	//���ô˽�Ŀ��Ƶ����Ƶ����
	inline void SetStreamIndex(int index, AVMediaType type);
	//��ȡ�˽�Ŀ��Ƶ����Ƶ����
	inline int  GetStreamIndex(AVMediaType type)const;
	//�򿪽�����
	bool FindOpenCodec(void);
	//����AVPacket��
	int SetPacket(AVPacket* pkt);
	//���������߳�
	bool StartDecoder(void);
	//ֹͣ�����߳�
	void StopDecoder(void);

	//friend CProgram* GetThis(){ return this; }
private:
	bool InnerOpenVideoCodec(void);
	bool InnerOpenAudioCodec(void);
	int  DecoderThread(); //�����߳�
private:
	int					m_id;
	int					m_videoIndex;
	int					m_audioIndex;
	AVCodecContext*		m_pAudioCodecCtx; //������������
	AVCodec*			m_pAudioCodec; //������
	AVCodecContext*		m_pVideoCodecCtx;
	AVCodec*			m_pVideoCodec;
	AVFormatContext*	m_fmt_ctx;

	std::deque<AVPacket>			m_pkt;
	std::shared_ptr<std::mutex>		m_pktMutex; //������
	std::shared_ptr<std::thread>	m_thread; //�����߳�
	bool							m_bRun;   //�߳��˳����
};

