// Temp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<iostream>
#include<string>
#include<queue>
#include<map>
using namespace std;

#define __STDC_CONSTANT_MACROS
extern "C"{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/error.h"
#include "libavutil/avutil.h"
}
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"swresample.lib")

void g_Error(int nCode, const char* pRest)
{
	char err[100] = { 0 };
	av_make_error_string(err, 100, nCode);
	cout << pRest << err << endl;
}

const AVRational G_AV_BASE_TIME_Q = { 1, AV_TIME_BASE };
struct stProgram 
{
	int programID;
	int videoIndex;
	int audioIndex;
}Program,*pPragram;


std::map<int, std::queue<AVPacket>> mapProgramPacket;

int _tmain(int argc, _TCHAR* argv[])
{
	string str = "E:/1234.ts"; //文件名
	AVFormatContext* fmt_ctx = NULL;
	AVDictionary* pDict = NULL;
	AVProgram* pProg = NULL;
	AVPacket packet = { 0 };
	//注册所有解码器
	av_register_all();
	//初始化网络
	avformat_network_init();

	av_dict_set(&pDict, "timeout", "6000", 0);
	int nRet = avformat_open_input(&fmt_ctx, str.c_str(), NULL, &pDict);
	if (nRet < 0)
	{
		g_Error(nRet, "avformat_open_input error. ");
		goto __Error;
	}
	
	nRet = avformat_find_stream_info(fmt_ctx, NULL);
	if (nRet <0)
	{
		g_Error(nRet, "avformat_find_stream_info");
		goto __Error;
	}
	//打印基本信息
	av_dump_format(fmt_ctx, -1, str.c_str(), 0);
	//流数量
	int streamNum = fmt_ctx->nb_streams;
	//节目数量
	int programNum = fmt_ctx->nb_programs;
	//章节数量
	int chapterNum = fmt_ctx->nb_chapters;

	int nIndex = 0;
	while (nIndex < programNum)
	{
		//pProg = av_find_program_from_stream(fmt_ctx, NULL, 0);
		pProg = fmt_ctx->programs[nIndex];
		cout << "Program ID: " << pProg->id << endl;
		cout << "Program Num: " << pProg->program_num << endl;
		cout << "nb_streams:  " << pProg->nb_stream_indexes << endl;
		for (unsigned int i = 0; i < pProg->nb_stream_indexes; ++i)
		{
			cout << "	stream index: " << pProg->stream_index[i] << endl;
		}
		cout << "pmt_id: " << pProg->pmt_pid << endl;
		cout << "pcr_id: " << pProg->pcr_pid << endl;
		cout << "start_pts: " << pProg->start_time << endl;
		double d = pProg->start_time * av_q2d(G_AV_BASE_TIME_Q);
		cout << "start_time: " << d << endl;
		cout << "end_pts: " << pProg->end_time << endl;
		d = pProg->end_time * av_q2d(G_AV_BASE_TIME_Q);
		cout << "end_time: " << d << endl;


		nIndex++;
	}
	
	//查找流信息
	av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1,NULL, 0);
	int readNum = 100;
	while (readNum>0)
	{
		av_read_frame(fmt_ctx, &packet);
		cout << packet.stream_index << endl;
		readNum--;
	}
	

__Error:
	if (fmt_ctx)
		avformat_close_input(&fmt_ctx);

	avformat_network_deinit();


	return 0;
}

