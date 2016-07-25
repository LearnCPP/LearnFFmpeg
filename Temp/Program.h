#pragma once
#include<deque>
#include<mutex>
#include<thread>
#include<memory>
#include "Decoder.h"
#include "D3DRender.h"
#include "Wave.h"

#define __STDC_CONSTANT_MACROS

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};




class CProgram
{
public:
	CProgram();
	~CProgram();
public:
	//��ʼ��
	int Init(int id, AVFormatContext* ic, RECT rt, HWND hWnd);
	//�ж���Ƶ������Ƶ�ǲ��Ǳ���Ŀ��
	inline bool IsProgram(int index) const;
	//���ô˽�Ŀ��Ƶ����Ƶ����
	void SetStreamIndex(int index, AVMediaType type);
	//��ȡ�˽�Ŀ��Ƶ����Ƶ����
	inline int  GetStreamIndex(AVMediaType type)const;
	//�򿪽�����
	bool FindOpenCodec(void);
	//����AVPacket��
	int SetPacket(AVPacket* pkt);
	//��ȡAVPacketbao
	AVPacket&& GetPacket(void);
	//���������߳�
	bool StartDecoder(void);
	//ֹͣ�����߳�
	void StopDecoder(void);

	//friend CProgram* GetThis(){ return this; }
private:
	bool InnerOpenVideoCodec(void);
	bool InnerOpenAudioCodec(void);
	int  DecoderThread(); //�����߳�
	int  DisplayVideoThread(); //��ʾ��Ƶ�߳�
	int  DisplayAudioThread(); //��ʾ��Ƶ�߳�
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
	//audio
	CDecoder						m_audio_dec;//��Ƶ������
	CWave							m_audio_play; //��Ƶ����
	WAVEFORMATEX					m_wavefmt; //��Ƶ���Žṹ��
	//video
	CDecoder						m_video_dec;//��Ƶ������
	CD3DRender						m_video_render; //��Ƶ��Ⱦ
	RECT							m_video_rt;	//��Ƶλ��
	HWND							m_hWnd; //���ھ��
};

