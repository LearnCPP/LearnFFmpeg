// FFDecoder.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "FFDecoder.h"
#include "common.h"
#include "WinLog.h"

//////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#include <io.h>
#include <fcntl.h>

extern "C"
{
#include<libavutil/opt.h>
}

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
	(*ppffCtx)->m_hReadPack = NULL;
	//(*ppffCtx)->m_hDisplayAudio = NULL;
	//(*ppffCtx)->m_hDisplayVideo = NULL;
	(*ppffCtx)->flag = 1;
}

int FFDECODER_API FF_Init_Context(FFContext* pffCtx, TCHAR* url, void* usedata, FUNC_GET_VIDEO video, 
	FUNC_GET_AUDIO audio, FUNC_GET_TS ts, bool bHasVideo, bool bHasAudio)
{
	int ret = 0;
	if ( pffCtx && url && usedata && (video || audio || ts) )
	{
		strcpy_s(pffCtx->url, MAX_PATH, url);

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

int FFDECODER_API FF_Start(FFContext* pffCtx)
{
	pffCtx->m_hReadPack = SDL_CreateThread(ReadPackThread, pffCtx);

	return 0;
}

int FFDECODER_API FF_Stop(FFContext* pffCtx)
{
	pffCtx->flag = 0;
	VideoState *vs = (VideoState*)pffCtx->is;
	packet_queue_abort(vs->aq);
	packet_queue_abort(vs->aq);

	if ( pffCtx->m_hReadPack )
	{
		Sleep(10);
		SDL_WaitThread(pffCtx->m_hReadPack, NULL);
		pffCtx->m_hReadPack = NULL;
	}

	
	if ( vs )
	{
		DestoryAudioQueue(&(vs->aq));
		DestoryVideoQueue(&(vs->vq));
	}
	
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
	DestoryAudioQueue((&is->aq));
	DestoryVideoQueue((&is->vq));
	av_free(is);
	is = NULL;

	free(pffCtx);
	pffCtx = NULL;

	CWinLog::ReleaseInstance();

	return 0;
}


int  DisplayAudioThread(VOID* pVoid)
{
	FFContext* ffctx = (FFContext*)pVoid;

	VideoState* is = (VideoState*)(ffctx->is);

	while (true && ffctx->flag != 0 )
	{
		if (ffctx->flag == -1)//解码结束
		{
			if ( is->aq->length<=0 && is->vq->length<=0 )//播放结束
			{
				break;
			}
		}

		//音频快与视频择加锁
// 		if (is->currentAudioClock > is->currentVideoClock + is->diffclock)
// 		{
// 			if (is->vq->length>0 && is->aq->length < AudioQueueMaxSize)
// 			{
// 				SDL_Delay(1);
// 				continue;
// 			}
// 		}
		AudioQueueItem* pAQI = (AudioQueueItem*)av_mallocz(sizeof(AudioQueueItem));
		double delay = 0;
		//从队列中取出数据
		if (GetAudioQueue(is->aq, pAQI))
		{
			FUNC_GET_AUDIO func = ffctx->m_get_audio;
			if (func && ffctx->flag!=0 )
			{
				FFAudioData ffad;
				ffad.data = pAQI->AudioData;
				ffad.length = pAQI->length;
				FFAudioInfo ffai;
				ffai.channels = pAQI->channels;
				ffai.bit_pre_sample = pAQI->bit_pre_sample;
				ffai.sample_rate = pAQI->sample_rate;
				
				delay = ffad.length * 1000 / (ffai.bit_pre_sample*ffai.channels*ffai.sample_rate / 8);
				//delay = pAQI->pts - is->currentAudioClock;
				//SDL_Delay(delay);
				func(ffctx->m_pUseData, &ffad, &ffai);
			}
			is->currentAudioClock = pAQI->pts; //设置音频当前时间
			//设置已播放时间
			ffctx->m_dwPlayTime = pAQI->pts;

			//SDL_Delay(delay);
			//Sleep(delay);

			av_free(pAQI->AudioData);
		}
		av_free(pAQI);

		SDL_Delay(delay);
	}

	return 0;
}

int  DisplayVideoThread(VOID* pVoid)
{
	FFContext* ffctx = (FFContext*)pVoid;
	VideoState* is = (VideoState*)ffctx->is;

	while (ffctx->flag != 0 && true)
	{
// 		if (ffctx->flag == -1) //解码完成
// 		{
// 			if (is->aq<=0 && is->vq<=0)
// 			{
// 				break;
// 			}
// 		}
		//当视频大于音频时，择等待
		if ( is->currentVideoClock > is->currentAudioClock+is->diffclock)
		{
			if ( is->aq->length>0 && is->vq->length<VideoQueueMaxSize )
			{//音频队中有数据，这样判断是因为，当只有视频时，视频满了，就可以播放了
				SDL_Delay(1);
				continue;
			}
		}
		VideoQueueItem* pVQI = (VideoQueueItem*)av_mallocz(sizeof(VideoQueueItem));
		if ( GetVideoQueue(is->vq, pVQI) )
		{
			double delay;
			FUNC_GET_VIDEO func = ffctx->m_get_video;
			if (func && ffctx->flag != 0)
			{
				FFVideoData ffvd;
				ffvd.pY = pVQI->Y;
				ffvd.linesizey = pVQI->lineszieY;
				ffvd.pU = pVQI->U;
				ffvd.linesizeu = pVQI->linesizeU;
				ffvd.pV = pVQI->V;
				ffvd.linesizev = pVQI->linesizeV;

				ffvd.timestamp = pVQI->pts;

				FFVideoInfo ffvi;
				ffvi.width = pVQI->width;
				ffvi.height = pVQI->height;

				delay = pVQI->pts - is->currentVideoClock;

				func(ffctx->m_pUseData, &ffvd, &ffvi);
			}
			is->currentVideoClock = pVQI->pts;
			SDL_Delay(delay);

			av_free(pVQI->Y);
			//av_free(pVQI->U);
			//av_free(pVQI->V);
		}
		av_free(pVQI);
	}

	return 0;
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

	if ( ret<=0 || g_ffctx==NULL || g_ffctx->flag==0 )
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
	AVCodec** ppCodec)
{
	int ret = 0;
	int idx = -1;
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

int  ReadPackThread(VOID* pVoid)
{
	FFContext* ffctx = (FFContext*)pVoid;

	SDL_Thread* m_hDisplayVideo = NULL;
	SDL_Thread* m_hDisplayAudio = NULL;

	VideoState *vs = (VideoState*)av_mallocz(sizeof(VideoState));
	memset(vs, 0, sizeof(VideoState));
	InitAudioQueue(&(vs->aq));
	InitVideoQueue(&(vs->vq));
	vs->diffclock = 0.2;

	AVInputFormat *iformat=NULL;
	AVCodec* pAudioCodec = NULL;
	AVCodec* pVideoCodec = NULL;
	AVCodecContext* pAuidoCodecCtx = NULL;
	AVCodecContext* pVideoCodecCtx = NULL;
	int audio_stream;
	int video_stream;

	AVFrame* pFrame = NULL;
	AVFrame* pRGBFrame = NULL;
	AVPacket packet;
	av_init_packet(&packet);
	packet.data = NULL;
	packet.size = 0;

	ffctx->is = (void*)vs;

	vs->ic = avformat_alloc_context();


	int err = 0; //

	//打开文件
	if ((err = avformat_open_input(&(vs->ic), ffctx->url, NULL, NULL)) < 0)
	{
		printf("打开url=%s出错,返回值err=%d", ffctx->url, err);
		goto err_return;
	}
	//avformat_find_stream_info
	err = avformat_find_stream_info(vs->ic, NULL);
	if ( err < 0 )
	{	
		//printf("avformat_find_stream_info err=%d, %s\n", err,av_err2str(err));
		char szError[200] = { 0 };
		av_strerror(err, szError, 200);
		printf("avformat_find_stream_info err=%d, %s\n", err, szError);
		goto err_return;
	}
	//打印基本信息
	av_dump_format(vs->ic, 0, ffctx->url, false);
	//
	g_ffctx = ffctx;

	//保存ts流的回调函数
	vs->ic->pb->read_packet = ff_read;

	//查找和打开解码器
	if ( ffctx->m_bAudio && find_open_decodecr(vs->ic,AVMEDIA_TYPE_AUDIO, &audio_stream,
		&pAuidoCodecCtx, &pAudioCodec) )
	{
		err = -1;
		goto err_return;
	}
	if ( ffctx->m_bVideo && find_open_decodecr(vs->ic,AVMEDIA_TYPE_VIDEO, &video_stream,
		&pVideoCodecCtx, &pVideoCodec) )
	{
		err = -1;
		goto err_return;
	}
	if (ffctx->m_bAudio)
	{
		vs->ic->streams[audio_stream]->discard = AVDISCARD_DEFAULT;
		AVStream *st = vs->ic->streams[audio_stream];
		double time = st->duration*av_q2d(st->time_base);

		int hour = (int)time / 3600;
		int minutes = ((int)time % 3600) / 60;
		int second = (int)time % 60;
		CString str;
		str.Format(_T("audio time = %02d:%02d:%02d"), hour,minutes,second);
		WriteRunLog(str);
	}
		
	if (ffctx->m_bVideo)
	{
		vs->ic->streams[video_stream]->discard = AVDISCARD_DEFAULT;
		AVStream *st = vs->ic->streams[video_stream];
		double time = st->duration*av_q2d(st->time_base);
		
		int hour = (int)time / 3600;
		int minutes = ((int)time % 3600) / 60;
		int second = (int)time % 60;
		CString str;
		str.Format(_T("video time = %02d:%02d:%02d"), hour, minutes, second);
		WriteRunLog(str);
		//设置播放总时间
		ffctx->m_dwTotalTime = time;
	}
		
	//音频帧转换格式
// 	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
// 	int out_sample_rate = 44100; //输出音频采样率
// 	int64_t input_channel_layout = av_get_default_channel_layout(pAuidoCodecCtx->channels);
// 	int out_channel = av_get_channel_layout_nb_channels(pAuidoCodecCtx->channel_layout);
// 	int out_nb_sample = 1024;
// 	int auidoLength = av_samples_get_buffer_size(NULL, out_channel, out_nb_sample, out_sample_fmt, 1);
// 	int sample = 0;

	struct SwrContext * audio_convert_ctx = NULL;
	if ( ffctx->m_bAudio && audio_stream != -1 && !audio_convert_ctx )
	{
// 		audio_convert_ctx = swr_alloc();
// 		audio_convert_ctx = swr_alloc_set_opts(audio_convert_ctx, out_channel, out_sample_fmt, out_nb_sample,
// 			input_channel_layout, pAuidoCodecCtx->sample_fmt, pAuidoCodecCtx->sample_rate, 0, NULL);
// 		swr_init(audio_convert_ctx);

		//开启音频显示线程
		m_hDisplayAudio = SDL_CreateThread(DisplayAudioThread, (void*)ffctx);
	}
	//视频格式转换
	struct SwsContext * img_convert_ctx = NULL;

	int videoLength = 0;
	if ( ffctx->m_bVideo && video_stream != -1 && !img_convert_ctx)
	{
		if (pVideoCodecCtx->pix_fmt != AV_PIX_FMT_NONE)
		{
			videoLength = avpicture_get_size(VideoType, pVideoCodecCtx->width, pVideoCodecCtx->height);
			img_convert_ctx = sws_getContext(pVideoCodecCtx->width, pVideoCodecCtx->height, pVideoCodecCtx->pix_fmt,
				pVideoCodecCtx->width, pVideoCodecCtx->height, VideoType, SWS_BICUBIC, NULL, NULL, NULL);
		}
		
		//开启视频显示线程
		m_hDisplayVideo = SDL_CreateThread(DisplayVideoThread, (void*)ffctx);
	}

	double currectFilePts = 0; //时间戳

	while ( ffctx->flag != 0 )
	{
		if ( !pFrame )
		{
			if (!(pFrame = av_frame_alloc()))
				return AVERROR(ENOMEM);
		}else{
			av_frame_unref(pFrame);
		}

		if (!packet.data)
		{
			av_free_packet(&packet);
			packet.data = NULL;
		}

		int result=0, gotframe=0;
		err = av_read_frame(vs->ic, &packet);
		if ( err < 0 )
		{
			if ( err==AVERROR_EXIT || err==AVERROR_EOF )
				break;

			continue;
		}

		if ( packet.stream_index == audio_stream )
		{//音频数据
			while ( packet.data && packet.size > 0 )
			{
				result = avcodec_decode_audio4(pAuidoCodecCtx, pFrame, &gotframe, &packet);
				double pts = 0;
				if (packet.dts == AV_NOPTS_VALUE && pFrame->opaque && *(int64_t*)pFrame->opaque != AV_NOPTS_VALUE)
				{
					pts = *(int64_t*)pFrame->opaque;
				}
// 				else if (packet.dts != AV_NOPTS_VALUE)
// 				{
// 					pts = packet.dts;
// 				}
				else if (packet.pts != AV_NOPTS_VALUE)
				{
					pts = packet.pts;
				}
				else
					pts = 0;
				pts *= av_q2d(vs->ic->streams[audio_stream]->time_base);

				packet.data += result;
				packet.size -= result;

				if (gotframe)
				{
					uint8_t* audiobuf = (uint8_t*)av_mallocz(MAX_AUDIO_FRAME_SIZE * 2);
					memset(audiobuf, 0, MAX_AUDIO_FRAME_SIZE * 2);
					if (NULL == audio_convert_ctx)
					{
						audio_convert_ctx = swr_alloc();
						av_opt_set_int(audio_convert_ctx, "in_channel_layout", pAuidoCodecCtx->channel_layout, 0);
						av_opt_set_int(audio_convert_ctx, "out_channel_layout", pAuidoCodecCtx->channel_layout, 0);
						av_opt_set_int(audio_convert_ctx, "in_sample_rate", pAuidoCodecCtx->sample_rate, 0);
						av_opt_set_int(audio_convert_ctx, "out_sample_rate", pAuidoCodecCtx->sample_rate, 0);
						av_opt_set_sample_fmt(audio_convert_ctx, "in_sample_fmt", (enum AVSampleFormat)pFrame->format, 0);
						av_opt_set_sample_fmt(audio_convert_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
						swr_init(audio_convert_ctx);
					}

					int bytes_per_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
					int data_size = pFrame->nb_samples * bytes_per_sample * pAuidoCodecCtx->channels;

					swr_convert(audio_convert_ctx, &audiobuf, pFrame->nb_samples,
						(const uint8_t**)pFrame->extended_data, pFrame->nb_samples);
					// 				swr_convert(*ppSwr, (uint8_t**)&pAudioBuffer, frame->nb_samples,
					// 					const_cast<const uint8_t**>(frame->extended_data), frame->nb_samples);
					//创建音频item
					AudioQueueItem *aqi = (AudioQueueItem*)av_mallocz(sizeof(AudioQueueItem));
					memset(aqi, 0, sizeof(AudioQueueItem));
					aqi->AudioData = audiobuf;
					aqi->length = data_size;

					aqi->channels = pAuidoCodecCtx->channels;
					aqi->bit_pre_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * 8;
					aqi->sample_rate = pAuidoCodecCtx->sample_rate;
					aqi->pts = pts;
					aqi->next = NULL;

					CString str;
					str.Format(_T("audio pts=%f\n"), aqi->pts);
					WriteRunLog(str);

					while (ffctx->flag != 0)
					{
						if ( PutAudioQueue(vs->aq, aqi)==0 )
							SDL_Delay(1);
						else
							break;
					}

					//av_free(audiobuf);
				}
			}//while	
		}
		else if ( packet.stream_index==video_stream)
		{//视频数据
			result = avcodec_decode_video2(pVideoCodecCtx, pFrame, &gotframe, &packet);
			double pts = 0;
			if (packet.dts == AV_NOPTS_VALUE && pFrame->opaque &&
				*(int64_t*)pFrame->opaque != AV_NOPTS_VALUE )
			{
				pts = *(int64_t*)pFrame->opaque;
			}
// 			else if ( packet.dts != AV_NOPTS_VALUE )
// 			{
// 				pts = packet.dts;
// 			}
			else if (packet.pts != AV_NOPTS_VALUE)
			{
				pts = packet.pts;
			}
			else
			{
				pts = 0;
			}
			pts *= av_q2d(vs->ic->streams[video_stream]->time_base);
			if ( gotframe )
			{
				if ( !pRGBFrame )
				{
					pRGBFrame = av_frame_alloc();
				}else{
					av_frame_unref(pRGBFrame);
				}
				uint8_t* videobuf = NULL;
				//创建视频Item
				VideoQueueItem *vqi;
				vqi = (VideoQueueItem*)av_mallocz(sizeof(VideoQueueItem));
				memset(vqi, 0, sizeof(VideoQueueItem));

				if ( img_convert_ctx )
				{
					videobuf = (uint8_t*)av_mallocz(videoLength);
					avpicture_fill((AVPicture*)pRGBFrame, videobuf, VideoType, pVideoCodecCtx->width,
						pVideoCodecCtx->height);
					sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pVideoCodecCtx->height,
						pRGBFrame->data, pRGBFrame->linesize);
					
					vqi->height = pVideoCodecCtx->height;
					vqi->width = pVideoCodecCtx->width;
					vqi->Y = pRGBFrame->data[0];
					vqi->lineszieY = pRGBFrame->linesize[0];
					vqi->U = pRGBFrame->data[1];
					vqi->linesizeU = pRGBFrame->linesize[1];
					vqi->V = pRGBFrame->data[2];
					vqi->linesizeV = pRGBFrame->linesize[2];
				}
				else
				{
					int size = avpicture_get_size((AVPixelFormat)pFrame->format, pFrame->width, pFrame->height);
					videobuf = (uint8_t*)av_mallocz(size);
					vqi->height = pVideoCodecCtx->height;
					vqi->width = pVideoCodecCtx->width;

					int size1 = vqi->height*pFrame->linesize[0];
					int size2 = vqi->height*pFrame->linesize[1] / 2;
					int size3 = vqi->height*pFrame->linesize[2] / 2;
					memcpy(videobuf, pFrame->data[0], size1);
					memcpy(videobuf + size1, pFrame->data[1], size2);
					memcpy(videobuf + size1 + size2, pFrame->data[2], size3);

					vqi->Y = videobuf;
					vqi->lineszieY = pFrame->linesize[0];
					vqi->U = videobuf + size1;
					vqi->linesizeU = pFrame->linesize[1];
					vqi->V = videobuf + size1 + size2;
					vqi->linesizeV = pFrame->linesize[2];
				}
				

				//currectFilePts = pts;
				vqi->pts = pts;
				vqi->next = NULL;

				CString str;
				str.Format(_T("video pts=%f\n"), vqi->pts);
				//WriteRunLog(str);
				//_tprintf(_T("%s"), str);

				//添加到队列中
				while (ffctx->flag != 0)
				{
					if (PutVideoQueue(vs->vq, vqi) == 0)
						SDL_Delay(1);
					else
						break;
				}

				//av_free(videobuf);
			}
		}
		av_frame_unref(pFrame);
//		av_free_packet(&packet);
	}


err_return:

	if ( pAuidoCodecCtx )
	{
		avcodec_close(pAuidoCodecCtx);
		pAuidoCodecCtx = NULL;
	}
	if ( pVideoCodecCtx )
	{
		avcodec_close(pVideoCodecCtx);
		pVideoCodecCtx = NULL;
	}
	if ( vs->ic )
	{
		avformat_close_input(&(vs->ic));
		avformat_free_context(vs->ic);
		vs->ic = NULL;
	}
	//关闭音频和视频显示线程
	if (vs->aq)
	{
		SDL_CondSignal(vs->aq->audioCond);
	}
	if ( vs->vq )
	{
		SDL_CondSignal(vs->vq->videoCond);
	}
	if ( m_hDisplayAudio )
	{
		SDL_WaitThread(m_hDisplayAudio, NULL);
		m_hDisplayAudio = NULL;
	}
	if ( m_hDisplayVideo )
	{
		SDL_WaitThread(m_hDisplayVideo, NULL);
		m_hDisplayVideo = NULL;
	}
	
	if ( pFrame )
	{
		av_frame_free(&pFrame);
		pFrame = NULL;
	}
	if ( pRGBFrame )
	{
		av_frame_free(&pRGBFrame);
		pRGBFrame = NULL;
	}

	return 0;
}
static int lockmgr(void **mtx, enum AVLockOp op)
{
	switch (op) {
	case AV_LOCK_CREATE:
		*mtx = SDL_CreateMutex();
		if (!*mtx)
			return 1;
		return 0;
	case AV_LOCK_OBTAIN:
		return !!SDL_LockMutex((SDL_mutex *)*mtx);
	case AV_LOCK_RELEASE:
		return !!SDL_UnlockMutex((SDL_mutex *)*mtx);
	case AV_LOCK_DESTROY:
		SDL_DestroyMutex((SDL_mutex *)*mtx);
		return 0;
	}
	return 1;
}
// 
// static int lockmgr(void **mtx, enum AVLockOp op)
// {
// 	SDL_mutex *pmtx;
// 	pmtx = (SDL_mutex *)(mtx);
// 	switch (op) {
// 	case AV_LOCK_CREATE:
// 		pmtx = SDL_CreateMutex();
// 		if (!pmtx)
// 			return 1;
// 		return 0;
// 	case AV_LOCK_OBTAIN:
// 		return !!SDL_LockMutex(pmtx);
// 	case AV_LOCK_RELEASE:
// 		return !!SDL_UnlockMutex(pmtx);
// 	case AV_LOCK_DESTROY:
// 		SDL_DestroyMutex(pmtx);
// 		return 0;
// 	}
// 	return 1;
// }

int FFDECODER_API FF_Global_init(void)
{
#if _DEBUG
	OpenConsole();
#endif
	//注册所有解码器
	av_register_all();
	//初始化网络
	avformat_network_init();
	//注册加锁，支持多线程
	if ( av_lockmgr_register(lockmgr) )
	{
		printf("av_lockmgr_register failed!\n");
		return -1;
	}

	printf("global init success\n");
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
	return pffCtx->m_dwTotalTime;
}

DWORD FFDECODER_API FF_PlayTime(FFContext* pffCtx)
{
	return pffCtx->m_dwPlayTime;
}
