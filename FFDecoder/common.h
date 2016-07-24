#pragma once
#include "stdafx.h"
//#include "DataQueue.h"
#include "pthread.h"
#include<assert.h>
#include<float.h>

//是否使用硬件加速
#define USE_HWACCEL_DXVA2 1

#if USE_HWACCEL_DXVA2
#include "dxva2.h"
#endif

/* 播放器状态. */
typedef enum play_status
{
	inited, playing, paused, completed, stoped
} play_status;

enum sync_type
{
	AV_SYNC_AUDIO_MASTER, /* 默认选择. */
	AV_SYNC_VIDEO_MASTER, /* 同步到视频时间戳. */
	AV_SYNC_EXTERNAL_CLOCK, /* 同步到外部时钟. */
};
/*队列类型*/
#define QUEUE_PACKET 0
#define QUEUE_AVFRAME 1


extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavformat/url.h"
#include "libavutil/time.h"
	//#include "libavutil/error.h"
};

//队列
typedef struct _av_queue
{
	void* m_first, *m_last;
	int m_size; //队列大小
	int m_type; //队列类型，AVpacket, AVFrame
	int abort_request; //退出标记
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
}av_queue;

/* 计算视频实时帧率和实时码率的时间单元. */
#define MAX_CALC_SEC 5

struct VideoState
{
	//AudioQueue *aq; //存储音频数据队列
	//VideoQueue *vq; //存储视频数据队列
	av_queue m_audio_pq; //存储音频packet队列
	av_queue m_video_pq; //存储视频packet队列
	av_queue m_audio_q; //存储音频AVframe队列
	av_queue m_video_q; //存储视频AVFrame队列

	/*渲染线程*/
	pthread_t m_audio_dec_thread;
	pthread_t m_video_dec_thread;
	pthread_t m_audio_render_thread;
	pthread_t m_video_render_thread;
	pthread_t m_read_pkt_thread;

	/*视音频重采样指针*/
	struct SwrContext *m_audio_swr_ctx; //音频转换指针
	struct SwsContext *m_video_sws_ctx; //视频转换指针
	//ReSampleContext *m_audio_resample_ctx; //音频重采样指针

	/* 音频和视频的AVStream、AVCodecContext指针和index.	*/
	AVCodecContext *m_audio_ctx;
	AVCodecContext *m_video_ctx;
	AVCodec *m_audio_codec;
	AVCodec *m_video_codec;

	int m_audio_index;
	int m_video_index;

	/* 读取数据包占用缓冲大小.	*/
	long volatile m_pkt_buffer_size;
	pthread_mutex_t m_buf_size_mtx;

	/* 同步类型. */
	int m_av_sync_type;

	/*
	* 用于计算视频播放时间
	* 即:  m_video_current_pts_drift = m_video_current_pts - time();
	*      m_video_current_pts是当前播放帧的pts时间, 所以在pts向前推进
	*      的同时, time也同样在向前推进, 所以pts_drift基本保存在一个
	*      time_base范围内浮动.
	* 播放时间 = m_video_current_pts_drift - time()
	*/
	double m_video_current_pts_drift;
	double m_video_current_pts;

	/* 以下变量用于计算音视频同步.	*/
	double m_frame_timer;
	double m_frame_last_pts;
	double m_frame_last_duration;
	double m_frame_last_delay;
	double m_frame_last_filter_delay;
	double m_frame_last_dropped_pts;
	double m_frame_last_returned_time;
	int64_t m_frame_last_dropped_pos;
	int64_t m_video_current_pos;
	int m_drop_frame_num;

	/* seek实现. */
	int m_seek_req;
	int m_seek_flags;
	int64_t m_seek_pos;
	int64_t m_seek_rel;
	int m_seek_by_bytes;
	int m_seeking;

	/* 最后一个解码帧的pts, 解码帧缓冲大小为2, 也就是当前播放帧的下一帧.	*/
	double m_audio_clock;
	double m_video_clock;
	double m_external_clock;
	double m_external_clock_time;

	/* 当前音频播放buffer大小.	*/
	uint32_t m_audio_buf_size;

	/* 当前音频已经播放buffer的位置.	*/
	uint32_t m_audio_buf_index;
	int32_t m_audio_write_buf_size;
	double m_audio_current_pts_drift;
	double m_audio_current_pts_last;

	/* 播放状态. */
	play_status m_play_status;
	int m_rendering;
	double m_duration;

	/* 实时视频输入位率. */
	int m_enable_calc_video_bite;
	int m_real_bit_rate;
	int m_read_bytes[MAX_CALC_SEC]; /* 记录5秒内的字节数. */
	int m_last_vb_time;
	int m_vb_index;

	/* 帧率. */
	int m_enable_calc_frame_rate;
	int m_real_frame_rate;
	int m_frame_num[MAX_CALC_SEC]; /* 记录5秒内的帧数. */
	int m_last_fr_time;
	int m_fr_index;

	AVFormatContext *m_format_ctx;

	int64_t start_time;	/* 起始时间. */

	int64_t video_start_time;	/* 视频起始时间信息. */
	int64_t audio_start_time;	/* 音频起始时间信息. */

	AVRational video_time_base;	/* 视频基本时间. */
	AVRational audio_time_base;	/* 音频基本时间. */

	AVRational video_frame_rate;	/* 视频帧率. */
	AVRational audio_frame_rate;	/* 音频帧率. */

	int width;		/* 视频宽. */
	int height;		/* 视频高. */

	int sample_rate;	/* 音频采样率. */
	int channels;		/* 音频声道. */

	int64_t duration;				/* 视频时长信息. */
	int64_t	file_size;				/* 文件长度. */

	/**/
	int m_current_play_index;
	double m_start_time;
	double m_buffering;

	int m_abort; //停止标志

#if USE_HWACCEL_DXVA2
	vlc_va_dxva2_t* m_dxva2;
	bool m_bIsInit;
#endif
};


#define  VideoType  AV_PIX_FMT_YUV420P
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio 

#define IO_BUFFER_SIZE			32768
#define MAX_PKT_BUFFER_SIZE		5242880
#define MIN_AUDIO_BUFFER_SIZE	MAX_PKT_BUFFER_SIZE /* 327680 */
#define MIN_AV_FRAMES			5
#define AUDIO_BUFFER_MAX_SIZE	(AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)
#define AVDECODE_BUFFER_SIZE	2
#define DEVIATION				6

#define AV_SYNC_THRESHOLD		0.01f
#define AV_NOSYNC_THRESHOLD		10.0f
#define AUDIO_DIFF_AVG_NB		20

#define SEEKING_FLAG			-1
#define NOSEEKING_FLAG			0


/* INT64最大最小取值范围. */
#ifndef INT64_MIN
#define INT64_MIN (-9223372036854775807LL - 1)
#endif
#ifndef INT64_MAX
#define INT64_MAX (9223372036854775807LL)
#endif

/* rgb和yuv互换. */
#define _r(c) ((c) & 0xFF)
#define _g(c) (((c) >> 8) & 0xFF)
#define _b(c) (((c) >> 16) & 0xFF)
#define _a(c) ((c) >> 24)

#define rgba2y(c)  ( (( 263*_r(c) + 516*_g(c) + 100*_b(c)) >> 10) + 16  )
#define rgba2u(c)  ( ((-152*_r(c) - 298*_g(c) + 450*_b(c)) >> 10) + 128 )
#define rgba2v(c)  ( (( 450*_r(c) - 376*_g(c) -  73*_b(c)) >> 10) + 128 )

#define MAX_TRANS   255
#define TRANS_BITS  8


typedef struct AVFrameList
{
	AVFrame pkt;
	struct AVFrameList *next;
} AVFrameList;
