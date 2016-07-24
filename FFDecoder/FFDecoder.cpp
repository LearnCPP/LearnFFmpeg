// FFDecoder.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "FFDecoder.h"
#include "common.h"
#include "WinLog.h"

//////////////////////////////////////////////////////////////////////////

extern "C"
{
#include<libavutil/opt.h>
}
#ifdef _DEBUG
#include <io.h>
#include <fcntl.h>

void OpenConsole(void)
{
	::AllocConsole();
	HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((intptr_t)handle, _O_TEXT);
	FILE * hf = _fdopen(hCrt, "w");
	*stdout = *hf;
}
#endif
//////////////////////////////////////////////////////////////////////////
//线程私有的全局变量
__declspec(thread) FFContext *g_ffctx = NULL;

//////////////////////////////////////////////////////////////////////////
AVPacket flush_pkt;
AVFrame  flush_frm;
int type_skip = 1; //视频帧是否被跳过

//////////////////////////////////////////////////////////////////////////
/* 队列操作. */
static void queue_init(av_queue *q);
static void queue_flush(av_queue *q);
static void queue_end(av_queue *q);

/* 入队出队列操作. */
static int get_queue(av_queue *q, void *p);
static int put_queue(av_queue *q, void *p);
static void chk_queue(FFContext *play, av_queue *q, int size);

static
void
queue_init(av_queue *q)
{
	q->abort_request = 0;
	q->m_first = q->m_last = NULL;
	q->m_size = 0;

	pthread_mutex_init(&q->m_mutex, NULL);
	pthread_cond_init(&q->m_cond, NULL);

	if (q->m_type == QUEUE_PACKET)
		put_queue(q, (void*)&flush_pkt);
	else if (q->m_type == QUEUE_AVFRAME)
		put_queue(q, (void*)&flush_frm);
}

static
void queue_flush(av_queue *q)
{
	if (q->m_size == 0)
		return;

	if (q->m_type == QUEUE_PACKET)
	{
		AVPacketList *pkt, *pkt1;
		pthread_mutex_lock(&q->m_mutex);
		for (pkt = (AVPacketList *)q->m_first; pkt != NULL; pkt = pkt1)
		{
			pkt1 = pkt->next;
			if (pkt->pkt.data != flush_pkt.data)
				av_free_packet(&pkt->pkt);
			av_freep(&pkt);
		}
		q->m_last = NULL;
		q->m_first = NULL;
		q->m_size = 0;
		pthread_mutex_unlock(&q->m_mutex);
	}
	else if (q->m_type == QUEUE_AVFRAME)
	{
		AVFrameList *pkt, *pkt1;
		pthread_mutex_lock(&q->m_mutex);
		for (pkt = (AVFrameList*)q->m_first; pkt != NULL; pkt = pkt1)
		{
			pkt1 = pkt->next;
			if (pkt->pkt.data[0] != flush_frm.data[0])
				av_free(pkt->pkt.data[0]);
			av_freep(&pkt);
		}
		q->m_last = NULL;
		q->m_first = NULL;
		q->m_size = 0;
		pthread_mutex_unlock(&q->m_mutex);
	}
}

static
void queue_end(av_queue *q)
{
	queue_flush(q);

	if (q->m_cond)
		pthread_cond_destroy(&q->m_cond);
	if (q->m_mutex)
		pthread_mutex_destroy(&q->m_mutex);
}

#define PRIV_OUT_QUEUE(T) \
	pthread_mutex_lock(&q->m_mutex); \
	for (;;) \
			{ \
		if (q->abort_request) \
						{ \
			ret = -1; \
			break; \
						} \
		pkt1 = (T)q->m_first; \
		if (pkt1) \
						{ \
			q->m_first = pkt1->next; \
			if (!q->m_first) \
				q->m_last = NULL; \
			q->m_size--; \
			*pkt = pkt1->pkt; \
			av_free(pkt1); \
			ret = 1; \
			break; \
						} \
								else \
		{ \
			pthread_cond_wait(&q->m_cond, &q->m_mutex); \
		} \
			} \
	pthread_mutex_unlock(&q->m_mutex);

static
int get_queue(av_queue *q, void *p)
{
	if (q->m_type == QUEUE_PACKET)
	{
		AVPacketList *pktlist1;
		AVPacket *pkt = (AVPacket*)p;
		int ret;
		//PRIV_OUT_QUEUE(AVPacketList*)
		pthread_mutex_lock(&q->m_mutex);
		for (;;)
		{
			if (q->abort_request)
			{
				ret = -1;
				break;
			}
			pktlist1 = (AVPacketList*)q->m_first;
			if (pktlist1)
			{
				q->m_first = pktlist1->next;
				if (!q->m_first)
					q->m_last = NULL;
				q->m_size--;
				*pkt = pktlist1->pkt;
				av_free(pktlist1);
				ret = 1;
				break;
			}
			else
			{
				pthread_cond_wait(&q->m_cond, &q->m_mutex);
			}
		}
		pthread_mutex_unlock(&q->m_mutex);
		return ret;
	}
	else if (q->m_type == QUEUE_AVFRAME)
	{
		AVFrameList *pkt1;
		AVFrame *pkt = (AVFrame*)p;
		int ret;
		//PRIV_OUT_QUEUE(AVFrameList*)
		pthread_mutex_lock(&q->m_mutex);
		for (;;)
		{
			if (q->abort_request)
			{
				ret = -1;
				break;
			}
			pkt1 = (AVFrameList*)q->m_first;
			if (pkt1)
			{
				q->m_first = pkt1->next;
				if (!q->m_first)
					q->m_last = NULL;
				q->m_size--;
				*pkt = pkt1->pkt;
				av_free(pkt1);
				ret = 1;
				break;
			}
			else
			{
				pthread_cond_wait(&q->m_cond, &q->m_mutex);
			}
		}
		pthread_mutex_unlock(&q->m_mutex);
		return ret;
	}
	return -1;
}

#define PRIV_PUT_QUEUE(type) \
	pkt1 = (type*)av_malloc(sizeof(type)); \
	if (!pkt1) \
		return -1; \
	pkt1->pkt = *pkt; \
	pkt1->next = NULL; \
	\
	pthread_mutex_lock(&q->m_mutex); \
	if (!q->m_last) \
		q->m_first = pkt1; \
				else \
		((type*) q->m_last)->next = pkt1; \
	q->m_last = pkt1; \
	q->m_size++; \
	pthread_cond_signal(&q->m_cond); \
	pthread_mutex_unlock(&q->m_mutex);

static
int put_queue(av_queue *q, void *p)
{
	if (q->m_type == QUEUE_PACKET)
	{
		AVPacketList *pktlist;
		AVPacket *pkt = (AVPacket*)p;
		/* duplicate the packet */
		if (pkt != &flush_pkt && av_dup_packet(pkt) < 0)
			return -1;
		//PRIV_PUT_QUEUE(AVPacketList)
		pktlist = (AVPacketList*)av_malloc(sizeof(AVPacketList));
		if (!pktlist) 
			return -1; 
		pktlist->pkt = *pkt; 
		pktlist->next = NULL; 
				
		pthread_mutex_lock(&q->m_mutex); 
		if (!q->m_last) 
			q->m_first = pktlist; 
		else 
			((AVPacketList*)q->m_last)->next = pktlist;
		q->m_last = pktlist; 
		q->m_size++; 
		pthread_cond_signal(&q->m_cond); 
		pthread_mutex_unlock(&q->m_mutex);
		return 0;
	}
	else if (q->m_type == QUEUE_AVFRAME)
	{
		AVFrameList *framelist;
		AVFrame *frame = (AVFrame*)p;
		//PRIV_PUT_QUEUE(AVFrameList)
		int nFramelist = sizeof(AVFrameList);
		framelist = (AVFrameList*)av_malloc(nFramelist/*sizeof(AVFrameList)*/);
		if (!framelist)
			return -1;
		framelist->pkt = *frame;
		framelist->next = NULL;

		pthread_mutex_lock(&q->m_mutex);
		if (!q->m_last)
			q->m_first = framelist;
		else
			((AVFrameList*)q->m_last)->next = framelist;
		q->m_last = framelist;
		q->m_size++;
		pthread_cond_signal(&q->m_cond);
		pthread_mutex_unlock(&q->m_mutex);

		return 0;
	}

	return -1;
}

//ffmpeg 中的源码，稍作修改
static inline int transfer_wrapper(URLContext *h, uint8_t *buf,
	int size, int size_min,
	int(*transfer_func)(URLContext *h,
	uint8_t *buf,
	int size))
{
	int ret, len;
	int fast_retries = 5;
	int64_t wait_since = 0;

	len = 0;
	while (len < size_min) {
		/*if (ff_check_interrupt(&h->interrupt_callback))
		return AVERROR_EXIT;*/
		ret = transfer_func(h, buf + len, size - len);
		if (ret == AVERROR(EINTR))
			continue;
		if (h->flags & AVIO_FLAG_NONBLOCK)
			return ret;
		if (ret == AVERROR(EAGAIN)) {
			ret = 0;
			if (fast_retries) {
				fast_retries--;
			}
			else {
				if (h->rw_timeout) {
					if (!wait_since)
						wait_since = av_gettime_relative();
					else if (av_gettime_relative() > wait_since + h->rw_timeout)
						return AVERROR(EIO);
				}
				av_usleep(1000);
			}
		}
		else if (ret < 1)
			return (ret < 0 && ret != AVERROR_EOF) ? ret : len;
		if (ret)
			fast_retries = FFMAX(fast_retries, 2);
		len += ret;
	}
	return len;
}

int ff_read(void* h, uint8_t* buf, int size)
{
	int ret = 0;
	//判断当前是否是在读数据
	if (!((URLContext*)h)->flags & AVIO_FLAG_READ)
		return AVERROR(EIO);
	ret = transfer_wrapper((URLContext*)h, buf, size, 1, ((URLContext*)h)->prot->url_read);

	if ( ret<=0 || g_ffctx==NULL || g_ffctx->m_abort!=0 )
	{
		return ret;
	}

	FUNC_GET_TS func_get_ts = g_ffctx->m_get_ts;
	if ( func_get_ts && g_ffctx )
	{
		func_get_ts(g_ffctx->m_pUseData, buf, size);
	}

	return ret;
}

int find_open_decodecr(AVFormatContext* avfctx, enum AVMediaType avmt, int* index, AVCodecContext** ppCodecCtx,
	AVCodec** ppCodec, void** hwaccel)
{
	int ret = 0;
	int idx = -1;
//	*hwaccel = NULL;
	bool bVideo = (avmt == AVMEDIA_TYPE_VIDEO);
	idx = av_find_best_stream(avfctx, avmt, -1, -1, NULL, 0);
	if ( idx < 0 || idx >= (int)avfctx->nb_streams )
	{
		fprintf(stderr, "Can't find %s stream!\n", bVideo ? "video" : "audio");
		ret = -1;
		goto fail;
	}

	*index = idx; //stream 索引

	*ppCodecCtx = avfctx->streams[idx]->codec; //AVCodecContext
	if ( !(*ppCodecCtx) )
	{
		fprintf(stderr, "Can't find %s codec context!\n", bVideo ? "video" : "audio");
		ret = -1;
		goto fail;
	}

	*ppCodec = avcodec_find_decoder((*ppCodecCtx)->codec_id); //查找解码器
	if ( !(*ppCodec) )
	{
		fprintf(stderr, "Can't find %s decoder!\n", bVideo ? "video" : "audio");
		ret = -1;
		goto fail;
	}
#if USE_HWACCEL_DXVA2
	if ((*ppCodecCtx)->codec_id ==AV_CODEC_ID_H264 
		|| (*ppCodecCtx)->codec_id == AV_CODEC_ID_VC1
		|| (*ppCodecCtx)->codec_id == AV_CODEC_ID_MPEG2VIDEO
		|| (*ppCodecCtx)->codec_id == AV_CODEC_ID_WMV3
		|| (*ppCodecCtx)->codec_id == AV_CODEC_ID_H265)
	{
		vlc_va_dxva2_t* ist = (vlc_va_dxva2_t*)av_malloc(sizeof(vlc_va_dxva2_t));
		memset(ist, 0, sizeof(vlc_va_dxva2_t));
		(*ppCodecCtx)->opaque = ist;
		(*ppCodecCtx)->get_format = get_format;
		(*ppCodecCtx)->get_buffer2 = get_buffer;
		(*ppCodecCtx)->thread_safe_callbacks = 1;
		(*ppCodecCtx)->thread_count = 1;
		(*hwaccel) = (void*)ist;
	}
#endif
	idx = avcodec_open2(*ppCodecCtx, *ppCodec, NULL); //打开解码器
	if ( idx < 0 )
	{
		fprintf(stderr, "Can't open %s decoder !\n", bVideo ? "video" : "audio"); 
		ret = -1;
		goto fail;
	}

fail:
	return ret;
}

// 复制AVRational.
static void avrational_copy(AVRational &src, AVRational &dst)
{
	dst.den = src.den;
	dst.num = src.num;
}
/* 读取数据线程.	*/
static void* read_pkt_thrd(void *param);
static void* video_dec_thrd(void *param);
static void* audio_dec_thrd(void *param);

/* 渲染线程. */
static void* audio_render_thrd(void *param);
static void* video_render_thrd(void *param);

void wait_for_threads(VideoState *vs)
{
	void *status = NULL;
	pthread_join(vs->m_read_pkt_thread, &status);
	if (vs->m_audio_index != -1)
		pthread_join(vs->m_audio_dec_thread, &status);
	if (vs->m_video_index != -1)
		pthread_join(vs->m_video_dec_thread, &status);
	if (vs->m_audio_index != -1)
		pthread_join(vs->m_audio_render_thread, &status);
	if (vs->m_video_index != -1)
		pthread_join(vs->m_video_render_thread, &status);
}

static int lockmgr(void **mtx, enum AVLockOp op)
{
	switch (op) {
	case AV_LOCK_CREATE:
		*mtx = SDL_CreateMutex();
		//pthread_mutex_init((pthread_mutex_t*)*mtx, NULL);
		if (!*mtx)
			return 1;
		return 0;
	case AV_LOCK_OBTAIN:
		return !!SDL_LockMutex((SDL_mutex *)*mtx);
		//return !pthread_mutex_lock((pthread_mutex_t*)*mtx);
	case AV_LOCK_RELEASE:
		return !!SDL_UnlockMutex((SDL_mutex *)*mtx);
		//return !pthread_mutex_unlock((pthread_mutex_t*)*mtx);
	case AV_LOCK_DESTROY:
		SDL_DestroyMutex((SDL_mutex *)*mtx);
		//pthread_mutex_destroy((pthread_mutex_t*)*mtx);
		return 0;
	}
	return 1;
}


static void audio_copy(VideoState *vs, AVFrame *dst, AVFrame* src)
{
	int nb_sample;
	int dst_buf_size;
	int out_channels;
	int bytes_per_sample = 0;

	dst->linesize[0] = src->linesize[0];
	*dst = *src;
	dst->data[0] = NULL;
	//AVFrame opaque 用于判断是否需要跳帧
	//dst->opaque = 0;
	dst->opaque = av_mallocz(sizeof(int));

	/* 备注: FFMIN(play->m_audio_ctx->channels, 2); 会有问题, 因为swr_alloc_set_opts的out_channel_layout参数. */
	//out_channels = vs->m_audio_ctx->channels;
	out_channels = vs->channels;
	bytes_per_sample = av_get_bytes_per_sample(vs->m_audio_ctx->sample_fmt);
	/* 备注: 由于 src->linesize[0] 可能是错误的, 所以计算得到的nb_sample会不正确, 直接使用src->nb_samples即可. */
	nb_sample = src->nb_samples;/* src->linesize[0] / play->m_audio_ctx->channels / bytes_per_sample; */
	bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
	dst_buf_size = nb_sample * bytes_per_sample * out_channels;
	dst->data[0] = (uint8_t*)av_malloc(dst_buf_size);
	assert(dst->data[0]);
	avcodec_fill_audio_frame(dst, out_channels, AV_SAMPLE_FMT_S16, dst->data[0], dst_buf_size, 0);

	/* 重采样到AV_SAMPLE_FMT_S16格式. */
	if (vs->m_audio_ctx->sample_fmt != AV_SAMPLE_FMT_S16)
	{
		if (!vs->m_audio_swr_ctx)
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

		if (vs->m_audio_swr_ctx)
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

static void video_copy(VideoState *vs, AVFrame *dst, AVFrame *src)
{
	uint8_t *buffer;
// 	int len = avpicture_get_size(PIX_FMT_YUV420P, vs->m_video_ctx->width,
// 		vs->m_video_ctx->height);
	int len = avpicture_get_size(PIX_FMT_YUV420P, vs->width,
		vs->height);
	*dst = *src;
	buffer = (uint8_t*)av_malloc(len);
	assert(buffer);

	avpicture_fill((AVPicture*)&(*dst), buffer, PIX_FMT_YUV420P,
	 	vs->width, vs->height);


#if USE_HWACCEL_DXVA2
	if (vs->m_bIsInit)
	{
		vs->m_video_sws_ctx = sws_getContext(vs->m_video_ctx->width,
			vs->m_video_ctx->height, (AVPixelFormat)src->format,
			vs->width, vs->height,
			PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	}
	else
	{
		vs->m_video_sws_ctx = sws_getContext(vs->m_video_ctx->width,
			vs->m_video_ctx->height, vs->m_video_ctx->pix_fmt,
			vs->width, vs->height,
			PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	}
#else
	vs->m_video_sws_ctx = sws_getContext(vs->m_video_ctx->width,
		vs->m_video_ctx->height, vs->m_video_ctx->pix_fmt,
		vs->width, vs->height,
		PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

#endif
	dst->linesize[0] = vs->width;
	dst->linesize[1] = vs->width/2;
	dst->linesize[2] = vs->width/2;
	sws_scale(vs->m_video_sws_ctx, src->data, src->linesize, 0,
		vs->m_video_ctx->height, dst->data, dst->linesize);

	sws_freeContext(vs->m_video_sws_ctx);
	vs->m_video_sws_ctx = NULL;
}

static
void update_video_pts(VideoState *vs, double pts, int64_t pos)
{
	double time = (double)av_gettime() / 1000000.0;
	/* update current video pts */
	vs->m_video_current_pts = pts;
	vs->m_video_current_pts_drift = vs->m_video_current_pts - time;
	vs->m_video_current_pos = pos;
	vs->m_frame_last_pts = pts;
}

static
double audio_clock(VideoState *vs)
{
	double pts;
	int hw_buf_size, bytes_per_sec;
	pts = vs->m_audio_clock;
	hw_buf_size = vs->m_audio_buf_size - vs->m_audio_buf_index;
	bytes_per_sec = 0;
	if (vs->m_audio_index >= 0 )
	{
		/* 固定为2通道, 采样格式为AV_SAMPLE_FMT_S16.*/
		bytes_per_sec = vs->sample_rate * 2 * FFMIN(vs->channels, 2);
	}

	if (fabs(vs->m_audio_current_pts_drift) <= DBL_EPSILON)
	{
		if (fabs((double)vs->start_time) > DBL_EPSILON)
			vs->m_audio_current_pts_drift = pts - vs->m_audio_current_pts_last;
		else
			vs->m_audio_current_pts_drift = pts;
	}

	if (bytes_per_sec)
		pts -= (double)hw_buf_size / bytes_per_sec;
	return pts - vs->m_audio_current_pts_drift;
}

static
double video_clock(VideoState *vs)
{
	if (vs->m_play_status == paused)
		return vs->m_video_current_pts;
	return vs->m_video_current_pts_drift + (double)av_gettime() / 1000000.0f;
}

static
double external_clock(VideoState *vs)
{
	int64_t ti;
	ti = av_gettime();
	return vs->m_external_clock + ((ti - vs->m_external_clock_time) * 1e-6);
}

static
double master_clock(VideoState *vs)
{
	double val;

	if (vs->m_av_sync_type == AV_SYNC_VIDEO_MASTER)
	{
		if (vs->m_audio_index >= 0)
			val = video_clock(vs);
		else
			val = audio_clock(vs);
	}
	else if (vs->m_av_sync_type == AV_SYNC_AUDIO_MASTER)
	{
		if (vs->m_audio_index >= 0)
			val = audio_clock(vs);
		else
			val = video_clock(vs);
	}
	else
	{
		val = external_clock(vs);
	}

	return val;
}

static
void chk_queue(VideoState *vs, av_queue *q, int size)
{
	/* 防止内存过大.	*/
	while (1)
	{
		pthread_mutex_lock(&q->m_mutex);
		if (q->m_size >= size && !vs->m_abort)
		{
			pthread_mutex_unlock(&q->m_mutex);
			av_usleep(4000);
		}
		else
		{
			pthread_mutex_unlock(&q->m_mutex);
			return;
		}
	}
}

void av_pause(VideoState *vs)
{
	/* 一直等待为渲染状态时才控制为暂停, 原因是这样可以在暂停时继续渲染而不至于黑屏. */
	while (!vs->m_rendering)
		av_usleep(1000);
	/* 更改播放状态. */
	vs->m_play_status = paused;
}

void av_resume(VideoState *vs)
{
	/* 更改播放状态. */
	vs->m_play_status = playing;
}

void av_seek(VideoState *vs, double fact)
{
	/* 正在seek中, 只保存当前sec, 在seek完成后, 再seek. */
	if (vs->m_seeking == SEEKING_FLAG ||
		(vs->m_seeking > NOSEEKING_FLAG && vs->m_seek_req))
	{
		vs->m_seeking = fact * 1000;
		return;
	}

	/* 正常情况下的seek. */
	if (fabs(vs->m_duration - 0) < DBL_EPSILON)
	{
		int64_t size = vs->file_size;
		if (!vs->m_seek_req)
		{
			vs->m_seek_req = 1;
			vs->m_seeking = SEEKING_FLAG;
			vs->m_seek_pos = fact * size;
			vs->m_seek_rel = 0;
			vs->m_seek_flags &= ~AVSEEK_FLAG_BYTE;
			vs->m_seek_flags |= AVSEEK_FLAG_BYTE;
		}
	}
	else
	{
		if (!vs->m_seek_req)
		{
			vs->m_seek_req = 1;
			vs->m_seeking = SEEKING_FLAG;
			vs->m_seek_pos = fact * vs->m_duration;
			vs->m_seek_rel = 0;
			vs->m_seek_flags &= ~AVSEEK_FLAG_BYTE;
			/* play->m_seek_flags |= AVSEEK_FLAG_BYTE; */
		}
	}
}


static
void* read_pkt_thrd(void *param)
{
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;

	int ret=0;
	FFContext *ffctx = (FFContext*)param;
	VideoState *vs = (VideoState*)ffctx->is;

	int last_paused = vs->m_play_status;

	// 起始时间不等于0, 则先seek至指定时间.
	if (fabs(ffctx->m_start_time) > 1.0e-6)
	{
		av_seek(vs, ffctx->m_start_time);
	}

	vs->m_buffering = 0.0f;
	vs->m_real_bit_rate = 0;

	for (; !ffctx->m_abort;)
	{
		/* Initialize optional fields of a packet with default values.	*/
	//	av_packet_unref(&packet);

		/* 如果暂定状态改变. */
		if (last_paused != vs->m_play_status)
		{
			last_paused = vs->m_play_status;
			if (vs->m_play_status == playing)
				av_read_play(vs->m_format_ctx);
			if (vs->m_play_status == paused)
				av_read_pause(vs->m_format_ctx);
		}

		/* 如果seek未完成又来了新的seek请求. */
		if (vs->m_seeking > NOSEEKING_FLAG)
			av_seek(vs, (double)vs->m_seeking / 1000.0f);

		if (vs->m_seek_req)
		{
			int64_t seek_target = vs->m_seek_pos * AV_TIME_BASE;
			int64_t seek_min = /*play->m_seek_rel > 0 ? seek_target - play->m_seek_rel + 2:*/ INT64_MIN;
			int64_t seek_max = /*play->m_seek_rel < 0 ? seek_target - play->m_seek_rel - 2:*/ INT64_MAX;
			int seek_flags = 0 & (~AVSEEK_FLAG_BYTE);
			int ns, hh, mm, ss;
			int tns, thh, tmm, tss;
			double frac = (double)vs->m_seek_pos / vs->m_duration;

			tns = vs->m_duration;
			thh = tns / 3600.0f;
			tmm = (tns % 3600) / 60.0f;
			tss = tns % 60;

			ns = frac * tns;
			hh = ns / 3600.0f;
			mm = (ns % 3600) / 60.0f;
			ss = ns % 60;

			seek_target = frac * vs->m_duration * AV_TIME_BASE;
			if (vs->start_time != AV_NOPTS_VALUE)
				seek_target += vs->start_time;

			if (vs->m_audio_index >= 0)
			{
				queue_flush(&vs->m_audio_pq);
				put_queue(&vs->m_audio_pq, &flush_pkt);
			}
			if (vs->m_video_index >= 0)
			{
				queue_flush(&vs->m_video_pq);
				put_queue(&vs->m_video_pq, &flush_pkt);
			}
			vs->m_pkt_buffer_size = 0;

			//ret = dc->seek_packet(dc, seek_target);
			ret = avformat_seek_file(vs->m_format_ctx, -1, seek_min, seek_target, seek_max, seek_flags); 
			if (ret < 0)
			{
				fprintf(stderr, "%s: error while seeking\n", ffctx->url);
			}

			printf("Seek to %2.0f%% (%02d:%02d:%02d) of total duration (%02d:%02d:%02d)\n",
				frac * 100, hh, mm, ss, thh, tmm, tss);

			vs->m_seek_req = 0;
		}

		/* 缓冲读满, 在这休眠让出cpu.	*/
		while (vs->m_pkt_buffer_size > MAX_PKT_BUFFER_SIZE && !vs->m_abort && !vs->m_seek_req)
			av_usleep(32000);
		if (vs->m_abort)
			break;

		/* Return 0 if OK, < 0 on error or end of file.	*/
		ret = av_read_frame(vs->m_format_ctx, &packet);
 
 		if (ret < 0)
 		{
 			if (vs->m_video_pq.m_size == 0 && vs->m_audio_pq.m_size == 0 &&
 				vs->m_video_q.m_size == 0 && vs->m_audio_q.m_size == 0)
 				vs->m_play_status = completed;
 			av_usleep(100000);
 			continue;
 		}
 
 		if (vs->m_play_status == completed)
 			vs->m_play_status = playing;
 
 		/* 更新缓冲字节数.	*/
 		if (packet.stream_index == vs->m_video_index || packet.stream_index == vs->m_audio_index)
 		{
 			pthread_mutex_lock(&vs->m_buf_size_mtx);
 			vs->m_pkt_buffer_size += packet.size;
 			vs->m_buffering = (double)vs->m_pkt_buffer_size / (double)MAX_PKT_BUFFER_SIZE;
 			pthread_mutex_unlock(&vs->m_buf_size_mtx);
 		}
 
 		/* 在这里计算码率.	*/
 		if (vs->m_enable_calc_video_bite)
 		{
 			int current_time = 0;
 			/* 计算时间是否足够一秒钟. */
 			if (vs->m_last_vb_time == 0)
 				vs->m_last_vb_time = (double)av_gettime() / 1000000.0f;
 			current_time = (double)av_gettime() / 1000000.0f;
 			if (current_time - vs->m_last_vb_time >= 1)
 			{
 				vs->m_last_vb_time = current_time;
 				if (++vs->m_vb_index == MAX_CALC_SEC)
 					vs->m_vb_index = 0;
 
 				/* 计算bit/second. */
 				do
 				{
 					int sum = 0;
 					int i = 0;
 					for (; i < MAX_CALC_SEC; i++)
 						sum += vs->m_read_bytes[i];
 					vs->m_real_bit_rate = ((double)sum / (double)MAX_CALC_SEC) * 8.0f / 1024.0f;
 				} while (0);
 				/* 清空. */
 				vs->m_read_bytes[vs->m_vb_index] = 0;
 			}
 
 			/* 更新读取字节数. */
 			vs->m_read_bytes[vs->m_vb_index] += packet.size;
 		}
 
 //		av_dup_packet(&packet);

 		if (packet.stream_index == vs->m_video_index)
 			put_queue(&vs->m_video_pq, &packet);
		else if (packet.stream_index == vs->m_audio_index)
			put_queue(&vs->m_audio_pq, &packet);
		else
		{
			av_free_packet(&packet);
		}
	}

	/* 设置为退出状态.	*/
	vs->m_abort = TRUE;
	ffctx->m_abort = TRUE;

	return NULL;
}

static
void* audio_dec_thrd(void *param)
{
	AVPacket pkt, pkt2;
	int ret, n;
	AVFrame avframe = { 0 }, avcopy = { 0 };
	FFContext* ffctx = (FFContext*)param;
	VideoState *vs = (VideoState*)ffctx->is;

	int64_t v_start_time = 0;
	int64_t a_start_time = 0;

	if (vs->m_audio_index>=0 && vs->m_video_index>=0 )
	{
		v_start_time = vs->video_start_time;
		a_start_time = vs->audio_start_time;
	}

	for (; !vs->m_abort;)
	{
		av_init_packet(&pkt);
		while (vs->m_play_status == paused && !vs->m_abort)
			av_usleep(10000);
		ret = get_queue(&vs->m_audio_pq, &pkt);
		if (ret != -1)
		{
			if (pkt.data == flush_pkt.data)
			{
				AVFrameList* lst = NULL;
				avcodec_flush_buffers(vs->m_audio_ctx);
				while (vs->m_audio_q.m_size && !vs->m_audio_q.abort_request)
					av_usleep(1000);
				pthread_mutex_lock(&vs->m_audio_q.m_mutex);
				lst = (AVFrameList*)vs->m_audio_q.m_first;
				for (; lst != NULL; lst = lst->next)
					lst->pkt.opaque = &type_skip;	/*type为1表示skip.*/
				pthread_mutex_unlock(&vs->m_audio_q.m_mutex);
				continue;
			}

			/* 使用pts更新音频时钟. */
			if (pkt.pts != AV_NOPTS_VALUE)
				vs->m_audio_clock = av_q2d(vs->audio_time_base) * (pkt.pts - v_start_time);

			if (fabs(vs->m_audio_current_pts_last) < 1.0e-6)
				vs->m_audio_current_pts_last = vs->m_audio_clock;

			/* 计算pkt缓冲数据大小. */
			pthread_mutex_lock(&vs->m_buf_size_mtx);
			vs->m_pkt_buffer_size -= pkt.size;
			pthread_mutex_unlock(&vs->m_buf_size_mtx);

			/* 解码音频. */
			pkt2 = pkt;

			av_frame_unref(&avframe);

			while (!vs->m_abort)
			{
				int got_frame = 0;
				ret = avcodec_decode_audio4(vs->m_audio_ctx, &avframe, &got_frame, &pkt2);
				if (ret < 0)
				{
					printf("Audio error while decoding one frame!!!\n");
					break;
				}
				pkt2.size -= ret;
				pkt2.data += ret;

				/* 不足一个帧, 并且packet中还有数据, 继续解码当前音频packet. */
				if (!got_frame && pkt2.size > 0)
					continue;

				/* packet中已经没有数据了, 并且不足一个帧, 丢弃这个音频packet. */
				if (pkt2.size == 0 && !got_frame)
					break;

				if (avframe.linesize[0] != 0 && avframe.data[0])
				{
					/* copy并转换音频格式. */
					audio_copy(vs, &avcopy, &avframe);

					/* 将计算的pts复制到avcopy.pts.  */
					memcpy(&avcopy.pts, &vs->m_audio_clock, sizeof(double));

					/* 计算下一个audio的pts值.  */
					n = 2 * FFMIN(vs->m_audio_ctx->channels, 2);

					vs->m_audio_clock += ((double)avcopy.linesize[0] / (double)(n * vs->m_audio_ctx->sample_rate));

					/* 如果不是以音频同步为主, 则需要计算是否移除一些采样以同步到其它方式.	*/
					if (vs->m_av_sync_type == AV_SYNC_EXTERNAL_CLOCK ||
						vs->m_av_sync_type == AV_SYNC_VIDEO_MASTER && vs->m_video_index>=0 )
					{
						/* 暂无实现.	*/
					}

					/* 防止内存过大.	*/
					chk_queue(vs, &vs->m_audio_q, AVDECODE_BUFFER_SIZE);

					/* 丢到播放队列中.	*/
					put_queue(&vs->m_audio_q, &avcopy);

					/* packet中数据已经没有数据了, 解码下一个音频packet. */
					if (pkt2.size <= 0)
						break;
				}
			}
			av_free_packet(&pkt);
		}
	}

	return NULL;
}

static
void* video_dec_thrd(void *param)
{
	AVPacket pkt, pkt2;
	AVFrame *avframe, avcopy;
	int got_picture = 0;
	int ret = 0;
	FFContext *ffctx = (FFContext *)param;
	VideoState *vs = (VideoState *)ffctx->is;

	int64_t v_start_time = 0;
	int64_t a_start_time = 0;

	avframe = av_frame_alloc();

	if (vs->m_audio_index>=0 && vs->m_video_index>=0)
	{
		v_start_time = vs->video_start_time;
		a_start_time = vs->audio_start_time;

		vs->m_video_ctx->width = vs->width;
		vs->m_video_ctx->height = vs->height;
	}

	for (; !vs->m_abort;)
	{
		av_init_packet(&pkt);
		while (vs->m_play_status == paused && !vs->m_abort)
			av_usleep(10000);
		ret = get_queue(&vs->m_video_pq, (AVPacket*)&pkt);
		if (ret != -1)
		{
			if (pkt.data == flush_pkt.data)
			{
				AVFrameList* lst = NULL;

				avcodec_flush_buffers(vs->m_video_ctx);

				while (vs->m_video_q.m_size && !vs->m_video_q.abort_request)
					av_usleep(1000);

				pthread_mutex_lock(&vs->m_video_q.m_mutex);
				lst = (AVFrameList*)vs->m_video_q.m_first;
				for (; lst != NULL; lst = lst->next)
				{
					lst->pkt.opaque = &type_skip; /* type为1表示skip. */
				}
					
				vs->m_video_current_pos = -1;
				vs->m_frame_last_dropped_pts = AV_NOPTS_VALUE;
				vs->m_frame_last_duration = 0;
				vs->m_frame_timer = (double)av_gettime() / 1000000.0f;
				vs->m_video_current_pts_drift = -vs->m_frame_timer;
				vs->m_frame_last_pts = AV_NOPTS_VALUE;
				pthread_mutex_unlock(&vs->m_video_q.m_mutex);

				continue;
			}

			pthread_mutex_lock(&vs->m_buf_size_mtx);
			vs->m_pkt_buffer_size -= pkt.size;
			pthread_mutex_unlock(&vs->m_buf_size_mtx);
			pkt2 = pkt;

			while (pkt2.size > 0 && !vs->m_abort)
			{
				ret = avcodec_decode_video2(vs->m_video_ctx, avframe, &got_picture, &pkt2);
				if (ret < 0)
				{
					printf("Video error while decoding one frame!!!\n");
					break;
				}
				if (got_picture)
					break;
				pkt2.size -= ret;
				pkt2.data += ret;
			}

			if (got_picture)
			{
#if USE_HWACCEL_DXVA2
				if (vs->m_bIsInit)
				{
					if (avframe->format == vs->m_dxva2->hwaccel_pix_fmt) {
						ret = dxva2_retrieve_data(vs->m_video_ctx, avframe);
					}
				}
#endif
				double pts1 = 0.0f;
				double frame_delay, pts;

				/*
				* 复制帧, 并输出为PIX_FMT_YUV420P.
				*/

				video_copy(vs, &avcopy, avframe);

				/*
				* 初始化m_frame_timer时间, 使用系统时间.
				*/
				if (vs->m_frame_timer == 0.0f)
					vs->m_frame_timer = (double)av_gettime() / 1000000.0f;

				/*
				* 计算pts值.
				*/
				pts1 = (avcopy.best_effort_timestamp - a_start_time) * av_q2d(vs->video_time_base);
				if (pts1 == AV_NOPTS_VALUE)
					pts1 = 0;
				pts = pts1;

				/* 如果以音频同步为主, 则在此判断是否进行丢包. */
				if (vs->m_audio_index>=0 &&
					((vs->m_av_sync_type == AV_SYNC_AUDIO_MASTER) || vs->m_av_sync_type == AV_SYNC_EXTERNAL_CLOCK))
				{
					pthread_mutex_lock(&vs->m_video_q.m_mutex);
					/*
					* 最后帧的pts是否为AV_NOPTS_VALUE 且 pts不等于0
					* 计算视频时钟和主时钟源的时间差.
					* 计算pts时间差, 当前pts和上一帧的pts差值.
					*/
					ret = 1;
					if (vs->m_frame_last_pts != AV_NOPTS_VALUE && pts)
					{
						double clockdiff = video_clock(vs) - master_clock(vs);
						double ptsdiff = pts - vs->m_frame_last_pts;

						/*
						* 如果clockdiff和ptsdiff同时都在同步阀值范围内
						* 并且clockdiff与ptsdiff之和与m_frame_last_filter_delay的差
						* 如果小于0, 则丢弃这个视频帧.
						*/
						if (fabs(clockdiff) < AV_NOSYNC_THRESHOLD && ptsdiff > 0
							&& ptsdiff < AV_NOSYNC_THRESHOLD
							&& clockdiff + ptsdiff - vs->m_frame_last_filter_delay < 0)
						{
							vs->m_frame_last_dropped_pos = pkt.pos;
							vs->m_frame_last_dropped_pts = pts;
							vs->m_drop_frame_num++;
							printf("\nDROP: %3d drop a frame of pts is: %.3f\n", vs->m_drop_frame_num, pts);
							ret = 0;
						}
					}
					pthread_mutex_unlock(&vs->m_video_q.m_mutex);
					if (ret == 0)
					{
						/* 丢掉该帧. */
						av_free(avcopy.data[0]);
						continue;
					}
				}

				/* 计录最后有效帧时间. */
				vs->m_frame_last_returned_time = (double)av_gettime() / 1000000.0f;
				/* m_frame_last_filter_delay基本都是0吧. */
				vs->m_frame_last_filter_delay = (double)av_gettime() / 1000000.0f
					- vs->m_frame_last_returned_time;
				/* 如果m_frame_last_filter_delay还可能大于1, 那么m_frame_last_filter_delay置0. */
				if (fabs(vs->m_frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0f)
					vs->m_frame_last_filter_delay = 0.0f;

				/*
				*	更新当前m_video_clock为当前解码pts.
				*/
				if (pts != 0)
					vs->m_video_clock = pts;
				else
					pts = vs->m_video_clock;

				/*
				*	计算当前帧的延迟时长.
				*/
				frame_delay = av_q2d(vs->m_video_ctx->time_base);
				frame_delay += avcopy.repeat_pict * (frame_delay * 0.5);

				/*
				* m_video_clock加上该帧延迟时长,
				* m_video_clock是估算出来的下一帧的pts.
				*/
				vs->m_video_clock += frame_delay;

				/*
				* 防止内存过大.
				*/
				chk_queue(vs, &vs->m_video_q, AVDECODE_BUFFER_SIZE);

				/* 保存frame_delay为该帧的duration, 保存到.pts字段中. */
				memcpy(&avcopy.pkt_dts, &frame_delay, sizeof(double));
				/* 保存pts. */
				memcpy(&avcopy.pts, &pts, sizeof(double));
				/* 保存pos, pos即是文件位置. */
				avcopy.pkt_pos = pkt.pos;
				/* type为0表示no skip. */
				//avcopy.opaque = 0;
				avcopy.opaque = av_mallocz(sizeof(int));

				/* 丢进播放队列.	*/
				put_queue(&vs->m_video_q, &avcopy);
			}
			av_free_packet(&pkt);
		}
	}
	av_free(avframe);
	return NULL;
}

static
void* audio_render_thrd(void *param)
{
	FFContext *ffctx = (FFContext *)param;
	VideoState *vs = (VideoState *)ffctx->is;

	AVFrame audio_frame;
	int audio_size = 0;
	int ret, temp, inited = 0;
	int bytes_per_sec;

	while (!vs->m_abort)
	{
		ret = get_queue(&vs->m_audio_q, &audio_frame);
		if (audio_frame.data[0] == flush_frm.data[0])
			continue;
		if (ret != -1)
		{
			if (!inited && ffctx->m_get_audio)
			{
				inited = 1;
				/* 更改播放状态. */
				vs->m_play_status = playing;

				bytes_per_sec = vs->sample_rate *
					FFMIN(vs->m_audio_ctx->channels, 2) * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
			}
			else if (!ffctx->m_get_audio)
			{
				av_free(audio_frame.data[0]);
				break;
			}

			if (*(int*)audio_frame.opaque == 1)
			{
				av_free(audio_frame.opaque); //释放
				av_free(audio_frame.data[0]);
				continue;
			}

			audio_size = audio_frame.linesize[0];
			//audio_size = audio_frame.nb_samples;
			/* 清空. */
			vs->m_audio_buf_size = audio_size;
			vs->m_audio_buf_index = 0;

			/* 已经开始播放, 清空seeking的状态. */
			if (vs->m_seeking == SEEKING_FLAG)
				vs->m_seeking = NOSEEKING_FLAG;

			while (audio_size > 0)
			{
				if (inited == 1 && ffctx->m_get_audio)
				{
					FFAudioInfo fai;
					fai.channels = vs->channels;
					fai.sample_rate = vs->sample_rate;
					fai.bit_pre_sample = 16;
					FFAudioData fad;
					fad.data = audio_frame.data[0] + vs->m_audio_buf_index;
					//fad.data = *audio_frame.extended_data + vs->m_audio_buf_index;
					fad.length = vs->m_audio_buf_size - vs->m_audio_buf_index;
			
					temp = ffctx->m_get_audio(ffctx->m_pUseData,&fad,&fai);

					//vs->m_audio_buf_index += temp;
					vs->m_audio_buf_index += fad.length;
					/* 如果缓冲已满, 则休眠一小会. */
					if (temp == 0)
					{
						if (vs->m_audio_q.m_size > 0)
						{
							if (*((int*)((AVFrameList*)vs->m_audio_q.m_last)->pkt.opaque) == 1)
								break;
						}
						int delay = fad.length * 1000 / bytes_per_sec;
						av_usleep(delay*1000);
						//av_usleep(10000);
					}
				}
				else
				{
					assert(0);
				}
				audio_size = vs->m_audio_buf_size - vs->m_audio_buf_index;
			}
			av_free(audio_frame.opaque);
			av_free(audio_frame.data[0]);
		}
	}
	return NULL;
}

static
void* video_render_thrd(void *param)
{
	FFContext* ffctx = (FFContext*)param;
	VideoState* vs = (VideoState*)ffctx->is;

	AVFrame video_frame;
	int ret = 0;
	int inited = 0;
	double sync_threshold;
	double current_pts;
	double last_duration;
	double duration;
	double delay = 0.0f;
	double time;
	double next_pts;
	double diff = 0.0f;
	int64_t frame_num = 0;
	double diff_sum = 0;
	double avg_diff = 0.0f;

	while (!vs->m_abort)
	{
		/* 如果视频队列为空 */
		if (vs->m_video_q.m_size == 0)
		{
			pthread_mutex_lock(&vs->m_video_q.m_mutex);
			/*
			* 如果最后丢弃帧的pts不为空, 且大于最后pts则
			* 使用最后丢弃帧的pts值更新其它相关的pts值.
			*/
			if (vs->m_frame_last_dropped_pts != AV_NOPTS_VALUE && vs->m_frame_last_dropped_pts > vs->m_frame_last_pts)
			{
				update_video_pts(vs, vs->m_frame_last_dropped_pts, vs->m_frame_last_dropped_pos);
				vs->m_frame_last_dropped_pts = AV_NOPTS_VALUE;
			}
			pthread_mutex_unlock(&vs->m_video_q.m_mutex);
		}
		/* 获得下一帧视频. */
		ret = get_queue(&vs->m_video_q, &video_frame);
		if (ret != -1)
		{
			// 状态为正在渲染.
			vs->m_rendering = 1;
			// 如果没有初始化渲染器, 则初始化渲染器.
			if (!inited && ffctx->m_get_video)
			{
				inited = 1;
				vs->m_play_status = playing;
			}

			if (video_frame.data[0] == flush_frm.data[0])
				continue;

			do {
				/* 判断是否skip. */
				if (*((int*)video_frame.opaque) == 1)
				{
					/* 跳过该帧. */
					break;
				}

				/* 计算last_duration. */
				memcpy(&current_pts, &video_frame.pts, sizeof(double));
				last_duration = current_pts - vs->m_frame_last_pts;
				if (last_duration > 0 && last_duration < 10.0)
				{
					/* 更新m_frame_last_duration. */
					vs->m_frame_last_duration = last_duration;
				}

				/* 更新延迟同步到主时钟源. */
				delay = vs->m_frame_last_duration;
				if ((vs->m_av_sync_type == AV_SYNC_EXTERNAL_CLOCK) ||
					(vs->m_av_sync_type == AV_SYNC_AUDIO_MASTER && vs->m_audio_index>=0 ))
				{
					diff = video_clock(vs) - master_clock(vs);
					sync_threshold = FFMAX(AV_SYNC_THRESHOLD, delay) * 0.75;
					if (fabs(diff) < AV_NOSYNC_THRESHOLD)
					{
						if (diff <= -sync_threshold)
							delay = 0.0f;
						else if (diff >= sync_threshold)
							delay = 2.0f * delay;
					}
					else
					{
						if (diff < 0.0f)
							delay = 0.0f;
						else
							av_usleep(1000);
					}
				}

				/* 得到当前系统时间. */
				time = (double)av_gettime() / 1000000.0f;

				/* 如果当前系统时间小于播放时间加延迟时间, 则过一会重试. */
				if (time < vs->m_frame_timer + delay)
				{
					av_usleep(1000);
					continue;
				}

				/* 更新m_frame_timer. */
				if (delay > 0.0f)
					vs->m_frame_timer += delay * FFMAX(1, floor((time - vs->m_frame_timer) / delay));

				pthread_mutex_lock(&vs->m_video_q.m_mutex);
				update_video_pts(vs, current_pts, video_frame.pkt_pos);
				pthread_mutex_unlock(&vs->m_video_q.m_mutex);

				/* 计算下一帧的时间.  */
				if (vs->m_video_q.m_size > 0)
				{
					memcpy(&next_pts, &(((AVFrameList*)vs->m_video_q.m_last)->pkt.pts), sizeof(double));
					duration = next_pts - current_pts;
				}
				else
				{
					memcpy(&duration, &video_frame.pkt_dts, sizeof(double));
				}

				if (vs->m_audio_index>=0 && time > vs->m_frame_timer + duration)
				{
					if (vs->m_video_q.m_size > 1)
					{
						pthread_mutex_lock(&vs->m_video_q.m_mutex);
						vs->m_drop_frame_num++;
						pthread_mutex_unlock(&vs->m_video_q.m_mutex);
						printf("\nDROP: %3d drop a frame of pts is: %.3f\n", vs->m_drop_frame_num, current_pts);
						break;
					}
				}

				if (diff < 1000)
				{
					frame_num++;
					diff_sum += fabs(diff);
					avg_diff = (double)diff_sum / frame_num;
				}
				/*printf("%7.3f A-V: %7.3f A: %7.3f V: %7.3f FR: %d/fps, VB: %d/kbps\r",
					master_clock(vs), diff, audio_clock(vs), video_clock(vs), vs->m_real_frame_rate, vs->m_real_bit_rate);*/
				printf("%7.3f A-V: %7.3f A: %7.3f V: %7.3f Apts: %7.3f, VB: %d/kbps\r",
					master_clock(vs), diff, audio_clock(vs), video_clock(vs), vs->m_audio_clock, vs->m_real_bit_rate);
				/*	在这里计算帧率.	*/
				if (vs->m_enable_calc_frame_rate)
				{
					int current_time = 0;
					/* 计算时间是否足够一秒钟. */
					if (vs->m_last_fr_time == 0)
						vs->m_last_fr_time = (double)av_gettime() / 1000000.0f;
					current_time = (double)av_gettime() / 1000000.0f;
					if (current_time - vs->m_last_fr_time >= 1)
					{
						vs->m_last_fr_time = current_time;
						if (++vs->m_fr_index == MAX_CALC_SEC)
							vs->m_fr_index = 0;

						/* 计算frame_rate. */
						do
						{
							int sum = 0;
							int i = 0;
							for (; i < MAX_CALC_SEC; i++)
								sum += vs->m_frame_num[i];
							vs->m_real_frame_rate = (double)sum / (double)MAX_CALC_SEC;
						} while (0);
						/* 清空. */
						vs->m_frame_num[vs->m_fr_index] = 0;
					}

					/* 更新读取字节数. */
					vs->m_frame_num[vs->m_fr_index]++;
				}

				/* 已经开始播放, 清空seeking的状态. */
				if (vs->m_seeking == SEEKING_FLAG)
					vs->m_seeking = NOSEEKING_FLAG;

				if (inited == 1 && ffctx->m_get_video)
				{
					FFVideoInfo fvi;
					fvi.width = vs->width;
					fvi.height = vs->height;
					FFVideoData fvd;
					fvd.pY = video_frame.data[0];
					fvd.pU = video_frame.data[1];
					fvd.pV = video_frame.data[2];
					fvd.linesizey = video_frame.linesize[0];
					fvd.linesizeu = video_frame.linesize[1];
					fvd.linesizev = video_frame.linesize[2];
					
					ffctx->m_get_video(ffctx->m_pUseData, &fvd, &fvi);

					if (delay != 0)
						av_usleep(4000);
				}
				break;
			} while (TRUE);

			/* 渲染完成. */
			vs->m_rendering = 0;

			/* 如果处于暂停状态, 则直接渲染窗口, 以免黑屏. */
			while (vs->m_play_status == paused && inited == 1 && ffctx->m_get_video && !vs->m_abort)
			{
				FFVideoInfo fvi;
				fvi.width = vs->width;
				fvi.height = vs->height;
				FFVideoData fvd;
				fvd.pY = video_frame.data[0];
				fvd.pU = video_frame.data[1];
				fvd.pV = video_frame.data[2];
				fvd.linesizey = video_frame.linesize[0];
				fvd.linesizeu = video_frame.linesize[1];
				fvd.linesizev = video_frame.linesize[2];

				ffctx->m_get_video(ffctx->m_pUseData, &fvd, &fvi);

				av_usleep(16000);
			}

			/* 释放视频帧缓冲. */
			av_free(video_frame.opaque); //用于判断是否跳帧
			av_free(video_frame.data[0]);
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// 导出函数实现
int FFDECODER_API FF_Global_init(void)
{
#if _DEBUG
//	OpenConsole();
#endif
	//注册所有解码器
	av_register_all();
	//初始化网络
	avformat_network_init();
	//注册加锁，支持多线程
	if (av_lockmgr_register(lockmgr))
	{
		printf("av_lockmgr_register failed!\n");
		return -1;
	}

	return 0;
}

void FFDECODER_API FF_Global_unint(void)
{
	av_lockmgr_register(NULL);
	//
	avformat_network_deinit();
}

DWORD FFDECODER_API FF_TotalTime(FFContext* pffCtx)
{
	VideoState * vs = (VideoState*)pffCtx->is;

	return vs->duration / AV_TIME_BASE;
}

DWORD FFDECODER_API FF_PlayTime(FFContext* pffCtx)
{
	VideoState * vs = (VideoState*)pffCtx->is;
	
	return master_clock(vs);
}

int FFDECODER_API FF_Start(FFContext* ffctx)
{
	VideoState *vs = (VideoState*)av_mallocz(sizeof(VideoState));
	memset(vs, 0, sizeof(VideoState));
	vs->m_audio_swr_ctx = NULL;
	vs->m_video_sws_ctx = NULL;

	vs->m_av_sync_type = AV_SYNC_AUDIO_MASTER; //

	vs->m_format_ctx = avformat_alloc_context();

	int ret = 0; //

	//打开文件
	if ((ret = avformat_open_input(&(vs->m_format_ctx), ffctx->url, NULL, NULL)) < 0)
	{
		printf("打开url=%s出错,返回值err=%d", ffctx->url, ret);
		goto err_return;
	}
	//avformat_find_stream_info
	ret = avformat_find_stream_info(vs->m_format_ctx, NULL);
	if (ret < 0)
	{
		//printf("avformat_find_stream_info err=%d, %s\n", err,av_err2str(err));
		char szError[200] = { 0 };
		av_strerror(ret, szError, 200);
		printf("avformat_find_stream_info err=%d, %s\n", ret, szError);
		goto err_return;
	}
	//打印基本信息
	av_dump_format(vs->m_format_ctx, -1, ffctx->url, 0);
	//
	g_ffctx = ffctx;

	//保存ts流的回调函数
	//vs->m_format_ctx->pb->read_packet = ff_read;

	//查找和打开解码器
	void* hwaccel = NULL;
	if (ffctx->m_bAudio && find_open_decodecr(vs->m_format_ctx, AVMEDIA_TYPE_AUDIO,
		&vs->m_audio_index, &vs->m_audio_ctx, &vs->m_audio_codec,NULL))
	{
		ret = -1;
		goto err_return;
	}
	if (ffctx->m_bVideo && find_open_decodecr(vs->m_format_ctx, AVMEDIA_TYPE_VIDEO,
		&vs->m_video_index, &vs->m_video_ctx, &vs->m_video_codec, &hwaccel))
	{
		ret = -1;
		goto err_return;
	}
#if USE_HWACCEL_DXVA2
	if (hwaccel)
	{
		vs->m_dxva2 = (vlc_va_dxva2_t*)hwaccel;
		vs->m_bIsInit = true;
	}
	else{
		vs->m_bIsInit = false;
	}
#endif
	//如果有音频，则保留一些音频的基本信息
	if (ffctx->m_bAudio && vs->m_audio_index >= 0)
	{
		AVStream *stream = vs->m_format_ctx->streams[vs->m_audio_index];
		stream->discard = AVDISCARD_DEFAULT;
 		vs->audio_start_time = stream->start_time;
 		avrational_copy(stream->r_frame_rate, vs->audio_frame_rate);
 		avrational_copy(stream->time_base, vs->audio_time_base);
 		vs->sample_rate = stream->codec->sample_rate;
		vs->channels = stream->codec->channels > 2 ? 2 : stream->codec->channels;//大于2时，就重采样成2声道
 		vs->m_audio_ctx = stream->codec;
	}
	//如果有视频，则保留一些视频的基本信息	
	if (ffctx->m_bVideo && vs->m_video_index >= 0)
	{
		AVStream *stream = vs->m_format_ctx->streams[vs->m_video_index];
		stream->discard = AVDISCARD_DEFAULT;
 		avrational_copy(stream->r_frame_rate, vs->video_frame_rate);
 		avrational_copy(stream->time_base, vs->video_time_base);
 		vs->video_start_time = stream->start_time;
		vs->width = ffctx->m_width >0 ? ffctx->m_width : stream->codec->width;
		vs->height = ffctx->m_height >0 ? ffctx->m_height : stream->codec->height;

 		vs->m_video_ctx = stream->codec;
	}

	vs->start_time = vs->m_format_ctx->start_time;
	vs->duration = vs->m_format_ctx->duration;
	vs->file_size = avio_size(vs->m_format_ctx->pb);

	/* 初始化各变量. */
	av_init_packet(&flush_pkt);
	flush_pkt.data = (uint8_t*)"FLUSH";
	flush_frm.data[0] = (uint8_t*)"FLUSH";
	ffctx->m_abort = 0;

	/* 初始化队列. */
	if (vs->m_audio_index != -1)
	{
		vs->m_audio_pq.m_type = QUEUE_PACKET;
		queue_init(&vs->m_audio_pq);
		vs->m_audio_q.m_type = QUEUE_AVFRAME;
		queue_init(&vs->m_audio_q);
	}
	if (vs->m_video_index != -1)
	{
		vs->m_video_pq.m_type = QUEUE_PACKET;
		queue_init(&vs->m_video_pq);
		vs->m_video_q.m_type = QUEUE_AVFRAME;
		queue_init(&vs->m_video_q);
	}

	/* 初始化读取文件数据缓冲计数mutex. */
	pthread_mutex_init(&vs->m_buf_size_mtx, NULL);


	pthread_attr_t attr; //线程熟性

	/* 计算时长. */
	vs->m_duration = (double)vs->duration / AV_TIME_BASE;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	ffctx->is = (void*)vs;
	
	/* 创建线程. */
	ret = pthread_create(&vs->m_read_pkt_thread, &attr, read_pkt_thrd,
		(void*)ffctx);
	if (ret)
	{
		printf("ERROR; return code from pthread_create() is %d\n", ret);
		return ret;
	}
	if (vs->m_audio_index != -1)
	{
		/*play->m_audio_current_pts_drift = av_gettime() / 1000000.0f;*/
		ret = pthread_create(&vs->m_audio_dec_thread, &attr, audio_dec_thrd,
			(void*)ffctx);
		if (ret)
		{
			printf("ERROR; return code from pthread_create() is %d\n", ret);
			goto err_return;
		}
	}
	if (vs->m_video_index != -1)
	{
		ret = pthread_create(&vs->m_video_dec_thread, &attr, video_dec_thrd,
			(void*)ffctx);
		if (ret)
		{
			printf("ERROR; return code from pthread_create() is %d\n", ret);
			goto err_return;
		}
	}
	if (vs->m_audio_index != -1)
	{
		ret = pthread_create(&vs->m_audio_render_thread, &attr, audio_render_thrd,
			(void*)ffctx);
		if (ret)
		{
			printf("ERROR; return code from pthread_create() is %d\n", ret);
			goto err_return;
		}
	}
	if (vs->m_video_index != -1)
	{
		ret = pthread_create(&vs->m_video_render_thread, &attr, video_render_thrd,
			(void*)ffctx);
		if (ret)
		{
			printf("ERROR; return code from pthread_create() is %d\n", ret);
			goto err_return;
		}
	}
	vs->m_play_status = playing;

	return ret;

err_return:
	if (vs->m_audio_ctx)
	{
		avcodec_close(vs->m_audio_ctx);
		vs->m_audio_ctx = NULL;
	}
	if (vs->m_video_ctx)
	{
		avcodec_close(vs->m_video_ctx);
		vs->m_video_ctx = NULL;
#if USE_HWACCEL_DXVA2
		if (vs->m_dxva2 && vs->m_bIsInit)
		{
			av_free(vs->m_dxva2);
			vs->m_dxva2 = NULL;
		}
#endif
	}
	if (vs->m_format_ctx)
	{
		avformat_close_input(&(vs->m_format_ctx));
		avformat_free_context(vs->m_format_ctx);
		vs->m_format_ctx = NULL;
	}

	return ret;
}


void FFDECODER_API FF_Alloc_Context(FFContext** ppffCtx)
{
	(*ppffCtx) = (FFContext*)malloc(sizeof(FFContext));
	memset(*ppffCtx, 0, sizeof(FFContext));

	(*ppffCtx)->url = (char*)malloc(MAX_PATH);
	memset((*ppffCtx)->url, 0, MAX_PATH);

	(*ppffCtx)->is = NULL;
	(*ppffCtx)->m_pUseData = NULL;
	(*ppffCtx)->m_bAudio = false;
	(*ppffCtx)->m_bVideo = false;
	(*ppffCtx)->m_get_audio = NULL;
	(*ppffCtx)->m_get_video = NULL;
	(*ppffCtx)->m_get_ts = NULL;
	
	(*ppffCtx)->m_abort = 0;
}

int FFDECODER_API FF_Init_Context(FFContext* pffCtx, TCHAR* url, void* usedata, FUNC_GET_VIDEO video,
	FUNC_GET_AUDIO audio, FUNC_GET_TS ts, bool bHasVideo, bool bHasAudio)
{
	int ret = 0;
	if (pffCtx && url && usedata && (video || audio || ts))
	{
		memcpy(pffCtx->url, url,MAX_PATH);

		pffCtx->m_pUseData = usedata;
		pffCtx->m_get_audio = audio;
		pffCtx->m_get_video = video;
		pffCtx->m_get_ts = ts;
		pffCtx->m_bAudio = bHasAudio;
		pffCtx->m_bVideo = bHasVideo;
	}
	else
		ret = -1;

	return ret;
}


int FFDECODER_API FF_Stop(FFContext* pffCtx)
{
	pffCtx->m_abort = TRUE;
	VideoState *vs = (VideoState*)pffCtx->is;
	vs->m_abort = TRUE;

	/* 通知各线程退出. */
	vs->m_audio_pq.abort_request = TRUE;
	pthread_cond_signal(&vs->m_audio_pq.m_cond);
	vs->m_video_pq.abort_request = TRUE;
	pthread_cond_signal(&vs->m_video_pq.m_cond);

	vs->m_audio_q.abort_request = TRUE;
	pthread_cond_signal(&vs->m_audio_q.m_cond);
	vs->m_video_q.abort_request = TRUE;
	pthread_cond_signal(&vs->m_video_q.m_cond);

	/* 先等线程退出, 再释放资源. */
	wait_for_threads(vs);

	queue_end(&vs->m_audio_q);
	queue_end(&vs->m_video_q);
	queue_end(&vs->m_audio_pq);
	queue_end(&vs->m_video_pq);

	/* 关闭解码器以及渲染器. */

	if (vs->m_audio_swr_ctx)
		swr_free(&vs->m_audio_swr_ctx);
	if (vs->m_video_sws_ctx)
		sws_freeContext(vs->m_video_sws_ctx);

	if (vs->m_buf_size_mtx)
		pthread_mutex_destroy(&vs->m_buf_size_mtx);

	/* 更改播放状态. */
	vs->m_play_status = stoped;

#if USE_HWACCEL_DXVA2
	if (vs->m_dxva2 && vs->m_bIsInit)
	{
		av_free(vs->m_dxva2);
		vs->m_dxva2 = NULL;
	}
#endif

	return 0;
}

int FFDECODER_API FF_Free_Context(FFContext* pffCtx)
{
	if (!pffCtx)
	{
		printf("FF_Free_Context, pffCtx has been release.");
		return -1;
	}

	free(pffCtx->url);
	pffCtx->url = NULL;

	VideoState* is = (VideoState*)pffCtx->is;

	av_free(is);
	is = NULL;

	free(pffCtx);
	pffCtx = NULL;

	CWinLog::ReleaseInstance();

	return 0;
}

void FFDECODER_API FF_SetVideoRect(FFContext* pffCtx, int width, int height)
{
	pffCtx->m_width = width;
	pffCtx->m_height = height;

	VideoState* vs = (VideoState*)pffCtx->is;

	if (vs)
	{
		vs->width = width;
		vs->height = height;

		queue_flush(&vs->m_video_q);
	}
}

void FFDECODER_API FF_Pause(FFContext* pffCtx, bool bPause)
{
	VideoState* vs = (VideoState*)pffCtx->is;

	if (!vs || vs->m_abort)
		return;

	if (bPause)
		av_pause(vs);
	else
		av_resume(vs);
}

void FFDECODER_API FF_Seek(FFContext* pffCtx, double seek)
{
	VideoState* vs = (VideoState*)pffCtx->is;

	if (!vs || vs->m_abort)
		return;

	av_seek(vs, seek);
}
