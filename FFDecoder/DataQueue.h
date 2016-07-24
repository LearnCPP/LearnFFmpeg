#pragma once

extern "C"{
#include "sdl/sdl.h"
#include "sdl/SDL_thread.h"
};

//��Ƶ����Ƶ���������,��С��
#define AudioQueueMaxSize  1000
#define AudioQueueMinSize  10
#define VideoQueueMaxSize  1000
#define VideoQueueMinSize  10


//�洢��Ƶ����
typedef struct AudioItem{
	uint8_t* AudioData;//��Ƶ����
	int length;//��Ƶ���ݳ���
	double pts; //ʱ���
	int channels; //������
	int bit_pre_sample; //��������
	int sample_rate; //������Ƶ����

	AudioItem* next;
}AudioQueueItem;
typedef struct  
{
	AudioQueueItem* first;//��ͷ
	AudioQueueItem* last; //��β
	int length; //���г���
	SDL_mutex* audioMutex; //����ͬ�������̵߳Ļ�����
	SDL_cond* audioCond; //�����߳�
	int abort_request;
}AudioQueue;
//��Ƶ����
typedef struct VideoItem
{
	uint8_t* Y; //YUV����Y����
	int lineszieY;
	uint8_t* U; //U����
	int linesizeU;
	uint8_t* V; //V����
	int linesizeV;

	int width; //��Ƶ���
	int height; //��Ƶ�߶�

	double pts; //ʱ���

	VideoItem* next;
}VideoQueueItem;
typedef struct 
{
	VideoQueueItem* first; //����ͷ
	VideoQueueItem* last; //����β
	int length; //���г���
	double buffer_pts; //����ʱ���
	SDL_mutex *videoMutex; //���������߳�ͬʱ�������еĻ�����
	SDL_cond *videoCond; //���ڻ����߳�
	int abort_request;
}VideoQueue;

//���в�����������

void InitAudioQueue(AudioQueue** aq); //��ʼ����Ƶ����
void InitVideoQueue(VideoQueue** vq); //��ʼ����Ƶ����

void ClearAudioQueue(AudioQueue* aq); //�����Ƶ����
void ClearVideoQueue(VideoQueue* vq); //�����Ƶ����

int PutAudioQueue(AudioQueue* aq, AudioQueueItem* aqi);//����Ƶ�����в�������
int PutVideoQueue(VideoQueue* vq, VideoQueueItem* vqi); //����Ƶ�����в�������

int GetAudioQueue(AudioQueue* aq, AudioQueueItem* aqi); //����Ƶ������ȡ������
int GetVideoQueue(VideoQueue* vq, VideoQueueItem* vqi); //����Ƶ������ȡ������

void packet_queue_abort(AudioQueue *q);
void packet_queue_abort(VideoQueue *q);

void DestoryAudioQueue(AudioQueue** aq); //���ٶ���
void DestoryVideoQueue(VideoQueue** vq); //���ٶ���