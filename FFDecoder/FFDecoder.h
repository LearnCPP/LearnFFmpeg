#pragma once

#ifdef __cplusplus
extern "C"{
#endif

#include "sdl/SDL.h"
#include "sdl/SDL_thread.h"
	//};

#include "stdint.h"
#include "stdafx.h"
	//typedef unsigned char  FFUCHAR;

#ifdef FFDECODER_EXPORTS
#define FFDECODER_API __declspec(dllexport)
#else
#define FFDECODER_API __declspec(dllimport)
#endif

	struct /*FFDECODER_API*/ FFVideoData
	{
		const uint8_t* pY; //Y
		const uint8_t* pU; //U
		const uint8_t* pV; //V
		int linesizey; //one line Y size
		int linesizeu; //one line U size
		int linesizev; //one line V size

		uint64_t timestamp; //display timestamp
	};

	struct /*FFDECODER_API*/ FFVideoInfo
	{
		int width; //video width
		int height; //video height
	};

	struct /*FFDECODER_API*/ FFAudioData{
		const uint8_t* data;
		int length;
	};

	struct /*FFDECODER_API*/ FFAudioInfo{
		int channels; //������
		int bit_pre_sample; //��������
		int sample_rate; //������Ƶ����
	};

	//���庯��ָ��
	typedef int(*FUNC_GET_VIDEO)(void*, const FFVideoData*, const FFVideoInfo*);
	typedef int(*FUNC_GET_AUDIO)(void*, const FFAudioData*, const FFAudioInfo*);
	typedef int(*FUNC_GET_TS)(void*, const uint8_t*, int);

	struct /*FFDECODER_API*/ FFContext{
		void* m_pUseData; //��ʶ��
		char * url; //��Ƶ���ַ���
		void* is; //�洢��Ƶ����Ƶ�ṹ
		bool m_bAudio; //�Ƿ�����Ƶ
		bool m_bVideo; //�Ƿ�����Ƶ

		DWORD m_dwTotalTime; //��Ƶ��ʱ��
		DWORD m_dwPlayTime; //����ʱ��

		FUNC_GET_VIDEO m_get_video;
		FUNC_GET_AUDIO m_get_audio;
		FUNC_GET_TS m_get_ts;

		double m_start_time; //��ʼʱ��
		int m_width; //��Ƶ���
		int m_height; //��Ƶ�߶�
		volatile int m_abort; //�̱߳��,1��ʾ�˳���0��ʾ����
	};
	//�����ڴ�ռ䣬������ʼ�� FFContext
	void FFDECODER_API FF_Alloc_Context(FFContext** ppffCtx);
	//���� FFContext
	int FFDECODER_API FF_Init_Context(FFContext* pffCtx, TCHAR* url, void* usedata, FUNC_GET_VIDEO video,
		FUNC_GET_AUDIO audio, FUNC_GET_TS ts, bool bHasVideo, bool bHasAudio);
	//���������߳�
	int FFDECODER_API FF_Start(FFContext* pffCtx);
	//ֹͣ�����߳�
	int FFDECODER_API FF_Stop(FFContext* pffCtx);
	//�ͷ�FFContext�ڴ�ռ�
	int FFDECODER_API FF_Free_Context(FFContext* pffCtx);

	//ȫ�ֳ�ʼ��
	int FFDECODER_API FF_Global_init(void);
	void FFDECODER_API FF_Global_unint(void);
	//������Ƶ����
	DWORD FFDECODER_API FF_TotalTime(FFContext* pffCtx);
	//�����Ѳ���ʱ��
	DWORD FFDECODER_API FF_PlayTime(FFContext* pffCtx);
	void FFDECODER_API FF_SetVideoRect(FFContext* pffCtx, int width, int height);
	void FFDECODER_API FF_Pause(FFContext* pffCtx,bool bPause);
	void FFDECODER_API FF_Seek(FFContext* pffCtx, double seek);

#ifdef __cplusplus
};
#endif