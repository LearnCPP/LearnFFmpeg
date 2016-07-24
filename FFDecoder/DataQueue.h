#pragma once

extern "C"{
#include "sdl/sdl.h"
#include "sdl/SDL_thread.h"
};

//音频，视频队列最大数,最小数
#define AudioQueueMaxSize  1000
#define AudioQueueMinSize  10
#define VideoQueueMaxSize  1000
#define VideoQueueMinSize  10


//存储音频队列
typedef struct AudioItem{
	uint8_t* AudioData;//音频数据
	int length;//音频数据长度
	double pts; //时间戳
	int channels; //声道数
	int bit_pre_sample; //采样精度
	int sample_rate; //采样（频）率

	AudioItem* next;
}AudioQueueItem;
typedef struct  
{
	AudioQueueItem* first;//对头
	AudioQueueItem* last; //队尾
	int length; //队列长度
	SDL_mutex* audioMutex; //用于同步两个线程的互斥量
	SDL_cond* audioCond; //唤醒线程
	int abort_request;
}AudioQueue;
//视频队列
typedef struct VideoItem
{
	uint8_t* Y; //YUV数据Y分量
	int lineszieY;
	uint8_t* U; //U分量
	int linesizeU;
	uint8_t* V; //V分量
	int linesizeV;

	int width; //视频宽度
	int height; //视频高度

	double pts; //时间戳

	VideoItem* next;
}VideoQueueItem;
typedef struct 
{
	VideoQueueItem* first; //队列头
	VideoQueueItem* last; //队列尾
	int length; //队列长度
	double buffer_pts; //缓冲时间戳
	SDL_mutex *videoMutex; //用于两个线程同时操作队列的互斥量
	SDL_cond *videoCond; //用于唤醒线程
	int abort_request;
}VideoQueue;

//队列操作函数声明

void InitAudioQueue(AudioQueue** aq); //初始化音频队列
void InitVideoQueue(VideoQueue** vq); //初始化视频队列

void ClearAudioQueue(AudioQueue* aq); //清空音频队列
void ClearVideoQueue(VideoQueue* vq); //清空视频队列

int PutAudioQueue(AudioQueue* aq, AudioQueueItem* aqi);//向音频队列中插入数据
int PutVideoQueue(VideoQueue* vq, VideoQueueItem* vqi); //向视频队列中插入数据

int GetAudioQueue(AudioQueue* aq, AudioQueueItem* aqi); //从音频队列中取出数据
int GetVideoQueue(VideoQueue* vq, VideoQueueItem* vqi); //从视频队列中取出数据

void packet_queue_abort(AudioQueue *q);
void packet_queue_abort(VideoQueue *q);

void DestoryAudioQueue(AudioQueue** aq); //销毁队列
void DestoryVideoQueue(VideoQueue** vq); //销毁队列