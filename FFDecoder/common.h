#pragma once
#include "stdafx.h"
//#include "DataQueue.h"
#include "pthread.h"
#include<assert.h>
#include<float.h>

//�Ƿ�ʹ��Ӳ������
#define USE_HWACCEL_DXVA2 1

#if USE_HWACCEL_DXVA2
#include "dxva2.h"
#endif

/* ������״̬. */
typedef enum play_status
{
	inited, playing, paused, completed, stoped
} play_status;

enum sync_type
{
	AV_SYNC_AUDIO_MASTER, /* Ĭ��ѡ��. */
	AV_SYNC_VIDEO_MASTER, /* ͬ������Ƶʱ���. */
	AV_SYNC_EXTERNAL_CLOCK, /* ͬ�����ⲿʱ��. */
};
/*��������*/
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

//����
typedef struct _av_queue
{
	void* m_first, *m_last;
	int m_size; //���д�С
	int m_type; //�������ͣ�AVpacket, AVFrame
	int abort_request; //�˳����
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
}av_queue;

/* ������Ƶʵʱ֡�ʺ�ʵʱ���ʵ�ʱ�䵥Ԫ. */
#define MAX_CALC_SEC 5

struct VideoState
{
	//AudioQueue *aq; //�洢��Ƶ���ݶ���
	//VideoQueue *vq; //�洢��Ƶ���ݶ���
	av_queue m_audio_pq; //�洢��Ƶpacket����
	av_queue m_video_pq; //�洢��Ƶpacket����
	av_queue m_audio_q; //�洢��ƵAVframe����
	av_queue m_video_q; //�洢��ƵAVFrame����

	/*��Ⱦ�߳�*/
	pthread_t m_audio_dec_thread;
	pthread_t m_video_dec_thread;
	pthread_t m_audio_render_thread;
	pthread_t m_video_render_thread;
	pthread_t m_read_pkt_thread;

	/*����Ƶ�ز���ָ��*/
	struct SwrContext *m_audio_swr_ctx; //��Ƶת��ָ��
	struct SwsContext *m_video_sws_ctx; //��Ƶת��ָ��
	//ReSampleContext *m_audio_resample_ctx; //��Ƶ�ز���ָ��

	/* ��Ƶ����Ƶ��AVStream��AVCodecContextָ���index.	*/
	AVCodecContext *m_audio_ctx;
	AVCodecContext *m_video_ctx;
	AVCodec *m_audio_codec;
	AVCodec *m_video_codec;

	int m_audio_index;
	int m_video_index;

	/* ��ȡ���ݰ�ռ�û����С.	*/
	long volatile m_pkt_buffer_size;
	pthread_mutex_t m_buf_size_mtx;

	/* ͬ������. */
	int m_av_sync_type;

	/*
	* ���ڼ�����Ƶ����ʱ��
	* ��:  m_video_current_pts_drift = m_video_current_pts - time();
	*      m_video_current_pts�ǵ�ǰ����֡��ptsʱ��, ������pts��ǰ�ƽ�
	*      ��ͬʱ, timeҲͬ������ǰ�ƽ�, ����pts_drift����������һ��
	*      time_base��Χ�ڸ���.
	* ����ʱ�� = m_video_current_pts_drift - time()
	*/
	double m_video_current_pts_drift;
	double m_video_current_pts;

	/* ���±������ڼ�������Ƶͬ��.	*/
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

	/* seekʵ��. */
	int m_seek_req;
	int m_seek_flags;
	int64_t m_seek_pos;
	int64_t m_seek_rel;
	int m_seek_by_bytes;
	int m_seeking;

	/* ���һ������֡��pts, ����֡�����СΪ2, Ҳ���ǵ�ǰ����֡����һ֡.	*/
	double m_audio_clock;
	double m_video_clock;
	double m_external_clock;
	double m_external_clock_time;

	/* ��ǰ��Ƶ����buffer��С.	*/
	uint32_t m_audio_buf_size;

	/* ��ǰ��Ƶ�Ѿ�����buffer��λ��.	*/
	uint32_t m_audio_buf_index;
	int32_t m_audio_write_buf_size;
	double m_audio_current_pts_drift;
	double m_audio_current_pts_last;

	/* ����״̬. */
	play_status m_play_status;
	int m_rendering;
	double m_duration;

	/* ʵʱ��Ƶ����λ��. */
	int m_enable_calc_video_bite;
	int m_real_bit_rate;
	int m_read_bytes[MAX_CALC_SEC]; /* ��¼5���ڵ��ֽ���. */
	int m_last_vb_time;
	int m_vb_index;

	/* ֡��. */
	int m_enable_calc_frame_rate;
	int m_real_frame_rate;
	int m_frame_num[MAX_CALC_SEC]; /* ��¼5���ڵ�֡��. */
	int m_last_fr_time;
	int m_fr_index;

	AVFormatContext *m_format_ctx;

	int64_t start_time;	/* ��ʼʱ��. */

	int64_t video_start_time;	/* ��Ƶ��ʼʱ����Ϣ. */
	int64_t audio_start_time;	/* ��Ƶ��ʼʱ����Ϣ. */

	AVRational video_time_base;	/* ��Ƶ����ʱ��. */
	AVRational audio_time_base;	/* ��Ƶ����ʱ��. */

	AVRational video_frame_rate;	/* ��Ƶ֡��. */
	AVRational audio_frame_rate;	/* ��Ƶ֡��. */

	int width;		/* ��Ƶ��. */
	int height;		/* ��Ƶ��. */

	int sample_rate;	/* ��Ƶ������. */
	int channels;		/* ��Ƶ����. */

	int64_t duration;				/* ��Ƶʱ����Ϣ. */
	int64_t	file_size;				/* �ļ�����. */

	/**/
	int m_current_play_index;
	double m_start_time;
	double m_buffering;

	int m_abort; //ֹͣ��־

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


/* INT64�����Сȡֵ��Χ. */
#ifndef INT64_MIN
#define INT64_MIN (-9223372036854775807LL - 1)
#endif
#ifndef INT64_MAX
#define INT64_MAX (9223372036854775807LL)
#endif

/* rgb��yuv����. */
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
