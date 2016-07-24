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
		int channels; //声道数
		int bit_pre_sample; //采样精度
		int sample_rate; //采样（频）率
	};

	//定义函数指针
	typedef int(*FUNC_GET_VIDEO)(void*, const FFVideoData*, const FFVideoInfo*);
	typedef int(*FUNC_GET_AUDIO)(void*, const FFAudioData*, const FFAudioInfo*);
	typedef int(*FUNC_GET_TS)(void*, const uint8_t*, int);

	struct /*FFDECODER_API*/ FFContext{
		void* m_pUseData; //标识符
		char * url; //视频流字符串
		void* is; //存储音频和视频结构
		bool m_bAudio; //是否有音频
		bool m_bVideo; //是否有视频

		DWORD m_dwTotalTime; //视频总时间
		DWORD m_dwPlayTime; //播放时间

		FUNC_GET_VIDEO m_get_video;
		FUNC_GET_AUDIO m_get_audio;
		FUNC_GET_TS m_get_ts;

		double m_start_time; //开始时间
		int m_width; //视频宽度
		int m_height; //视频高度
		volatile int m_abort; //线程标记,1表示退出，0表示正常
	};
	//申请内存空间，并做初始化 FFContext
	void FFDECODER_API FF_Alloc_Context(FFContext** ppffCtx);
	//设置 FFContext
	int FFDECODER_API FF_Init_Context(FFContext* pffCtx, TCHAR* url, void* usedata, FUNC_GET_VIDEO video,
		FUNC_GET_AUDIO audio, FUNC_GET_TS ts, bool bHasVideo, bool bHasAudio);
	//启动解码线程
	int FFDECODER_API FF_Start(FFContext* pffCtx);
	//停止解码线程
	int FFDECODER_API FF_Stop(FFContext* pffCtx);
	//释放FFContext内存空间
	int FFDECODER_API FF_Free_Context(FFContext* pffCtx);

	//全局初始化
	int FFDECODER_API FF_Global_init(void);
	void FFDECODER_API FF_Global_unint(void);
	//返回视频长度
	DWORD FFDECODER_API FF_TotalTime(FFContext* pffCtx);
	//返回已播放时间
	DWORD FFDECODER_API FF_PlayTime(FFContext* pffCtx);
	void FFDECODER_API FF_SetVideoRect(FFContext* pffCtx, int width, int height);
	void FFDECODER_API FF_Pause(FFContext* pffCtx,bool bPause);
	void FFDECODER_API FF_Seek(FFContext* pffCtx, double seek);

#ifdef __cplusplus
};
#endif