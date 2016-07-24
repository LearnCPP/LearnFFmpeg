#include "stdafx.h"
#include "DataQueue.h"
#include "common.h"

void InitAudioQueue(AudioQueue** aq)
{
	(*aq) = (AudioQueue*)malloc(sizeof(AudioQueue));
	memset(*aq, 0, sizeof(AudioQueue));

	(*aq)->audioMutex = SDL_CreateMutex();
	(*aq)->audioCond = SDL_CreateCond();
}

void InitVideoQueue(VideoQueue** vq)
{
	(*vq) = (VideoQueue*)malloc(sizeof(VideoQueue));
	memset(*vq, 0, sizeof(VideoQueue));

	(*vq)->videoMutex = SDL_CreateMutex();
	(*vq)->videoCond = SDL_CreateCond();
}

void ClearAudioQueue(AudioQueue* aq)
{
	AudioQueueItem *item, *temp;

	SDL_LockMutex(aq->audioMutex);
	for (item = aq->first; item != NULL; item=temp)
	{
		temp = item->next;
		av_free(item->AudioData);
		av_free(item);
		aq->length--;
	}
	aq->first = NULL;
	aq->last = NULL;
	SDL_UnlockMutex(aq->audioMutex);
}

void ClearVideoQueue(VideoQueue* vq)
{
	VideoQueueItem *item, *temp;

	SDL_LockMutex(vq->videoMutex);
	for (item = vq->first; item != NULL; item=temp)
	{
		temp = item->next;

		av_free(item->Y);
		//free(item->U);
		//free(item->V);
		av_free(item);

		vq->length--;
	}
	vq->first = NULL;
	vq->last = NULL;

	SDL_UnlockMutex(vq->videoMutex);
}

int PutAudioQueue(AudioQueue* aq, AudioQueueItem* aqi)
{
	int result = 0;
	if (aq->abort_request == 1)
		return -1;

	SDL_LockMutex(aq->audioMutex);
	if (aq->length < AudioQueueMaxSize)
	{
		if ( !aq->first )//如果队列为空
		{
			aq->first = aqi;
			aq->last = aqi;
			aq->length = 1;
		}
		else
		{
			aq->last->next = aqi; //添加到队列后面
			aq->length++;
			aq->last = aqi; //把当前变为队尾
		}
		if ( aq->length >= AudioQueueMinSize )
		{
			SDL_CondSignal(aq->audioCond); //唤醒其他线程，如果缓冲区里面有数据后唤醒，较好
		}
		result = 1;
	}
	else
	{//如果音频队列大于设定值，则让线程等待
		SDL_CondWait(aq->audioCond, aq->audioMutex);
	}
	SDL_UnlockMutex(aq->audioMutex);

	return result;
}

int PutVideoQueue(VideoQueue* vq, VideoQueueItem* vqi)
{
	int result = 0;
	if ( vq->abort_request==1 )
	{
		return -1;
	}

	SDL_LockMutex(vq->videoMutex);
	if ( vq->length < VideoQueueMaxSize )
	{
		if ( !vq->first ) //如果队列头结点为空，则为空队列
		{
			vq->first = vqi;
			vq->last = vqi;
			vq->length = 1;
			vq->buffer_pts = vqi->pts;
		}
		else
		{
			vq->last->next = vqi;
			vq->length++;
			vq->last = vqi;
			vq->buffer_pts = vqi->pts;
		}
		if ( vq->length <= VideoQueueMinSize )
		{
			SDL_CondSignal(vq->videoCond); //小于设定最小值，则唤醒下载线程
		}
		result = 1;
	}
	else
	{//如果视频帧缓冲区，比设定值大，则让线程等待
		SDL_CondWait(vq->videoCond, vq->videoMutex);
	}
	SDL_UnlockMutex(vq->videoMutex);

	return result;
}

int GetAudioQueue(AudioQueue* aq, AudioQueueItem* aqi)
{
	int result = 0;

	if (aq->abort_request == 1)
		return -1;

	SDL_LockMutex(aq->audioMutex);
	if ( aq->length > 0 )
	{
		if ( aq->first ) //如果有数据
		{
			*aqi = *(aq->first);
			if ( !aq->first->next )//只有一个
			{
				aq->first = NULL;
				aq->last = NULL;
			}
			else
			{
				aq->first = aq->first->next;
			}
			aq->length--;
			aqi->next = NULL;
			result = 1;
		}
		if ( aq->length <= AudioQueueMinSize )
		{
			SDL_CondSignal(aq->audioCond); //唤醒下载线程
		}
	}
	else
	{
		SDL_CondWait(aq->audioCond, aq->audioMutex); //解锁，等待被唤醒
	}
	SDL_UnlockMutex(aq->audioMutex);

	return result;
}

int GetVideoQueue(VideoQueue* vq, VideoQueueItem* vqi)
{
	int result = 0;

	if (vq->abort_request == 1)
		return -1;

	SDL_LockMutex(vq->videoMutex);
	if ( vq->length > 0 )
	{
		if ( vq->first )//有数据
		{
			*vqi = *(vq->first);
			if ( !vq->first->next )
			{
				vq->first = NULL;
				vq->last = NULL;
			}
			else
			{
				vq->first = vq->first->next;
			}
			vq->length--;
			vqi->next = NULL;
			result = 1;
		}
		if ( vq->length <= VideoQueueMinSize )
		{
			SDL_CondSignal(vq->videoCond); //唤醒下载线程
		}
	}
	else
	{
		SDL_CondWait(vq->videoCond, vq->videoMutex); //解锁，等待唤醒
	}
	SDL_UnlockMutex(vq->videoMutex);
	return result;
}

void DestoryAudioQueue(AudioQueue** aq)
{
	if (!(*aq))
		return;

	ClearAudioQueue(*aq);
	
	SDL_DestroyMutex((*aq)->audioMutex);
	SDL_DestroyCond((*aq)->audioCond);
	free(*aq);
	(*aq) = NULL;
}

void DestoryVideoQueue(VideoQueue** vq)
{
	if (!(*vq))
		return;
	ClearVideoQueue((*vq));
	SDL_DestroyMutex((*vq)->videoMutex);
	SDL_DestroyCond((*vq)->videoCond);
	free((*vq));
	(*vq) = NULL;
}

void packet_queue_abort(AudioQueue *q)
{
	SDL_LockMutex(q->audioMutex);

	q->abort_request = 1;

	SDL_CondSignal(q->audioCond);

	SDL_UnlockMutex(q->audioMutex);
}

void packet_queue_abort(VideoQueue *q)
{
	SDL_LockMutex(q->videoMutex);

	q->abort_request = 1;

	SDL_CondSignal(q->videoCond);

	SDL_UnlockMutex(q->videoMutex);
}