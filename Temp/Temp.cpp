// Temp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<iostream>
#include<string>
#include<list>
#include<map>
#include<windows.h>
#include "Program.h"

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

std::list<CProgram*>g_Program;
string g_str = "E:/1234.ts"; //文件名
AVFormatContext* g_fmt_ctx = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE hInstance;
	hInstance = GetModuleHandle(NULL);
	WNDCLASS Draw;
	Draw.cbClsExtra = 0;
	Draw.cbWndExtra = 0;
	Draw.hCursor = LoadCursor(hInstance, IDC_ARROW);;
	Draw.hIcon = LoadIcon(hInstance, IDI_APPLICATION);;
	Draw.lpszMenuName = NULL;
	Draw.style = CS_HREDRAW | CS_VREDRAW;
	Draw.hbrBackground = (HBRUSH)COLOR_WINDOW;
	Draw.lpfnWndProc = WindowProc;
	Draw.lpszClassName = _T("Play");
	Draw.hInstance = hInstance;


	RegisterClass(&Draw);

	HWND hwnd = CreateWindow(
		_T("Play"),           //上面注册的类名，要完全一致    
		_T("播放器"),  //窗口标题文字    
		WS_OVERLAPPEDWINDOW, //窗口外观样式    
		30,             //窗口相对于父级的X坐标    
		20,             //窗口相对于父级的Y坐标    
		480,                //窗口的宽度    
		250,                //窗口的高度    
		NULL,               //没有父窗口，为NULL    
		NULL,               //没有菜单，为NULL    
		hInstance,          //当前应用程序的实例句柄    
		NULL);              //没有附加数据，为NULL    

	// 显示窗口    
	ShowWindow(hwnd, SW_SHOW);
	// 更新窗口    
	UpdateWindow(hwnd);

	AVDictionary* pDict = NULL;
	AVPacket packet = { 0 };
	//注册所有解码器
	av_register_all();
	//初始化网络
	avformat_network_init();

	av_dict_set(&pDict, "timeout", "6000", 0);
	int nRet = avformat_open_input(&g_fmt_ctx, g_str.c_str(), NULL, &pDict);
	if (nRet < 0)
	{
		g_Error(nRet, "avformat_open_input error. ");
		goto __Error;
	}

	nRet = avformat_find_stream_info(g_fmt_ctx, NULL);
	if (nRet < 0)
	{
		g_Error(nRet, "avformat_find_stream_info");
		goto __Error;
	}
	//打印基本信息
	av_dump_format(g_fmt_ctx, -1, g_str.c_str(), 0);
	//流数量
	int streamNum = g_fmt_ctx->nb_streams;
	//节目数量
	int programNum = g_fmt_ctx->nb_programs;
	//章节数量
	int chapterNum = g_fmt_ctx->nb_chapters;

	int left = 0;
	int top = 0;
	int w = 480;
	int h = 320;
	RECT rt[6] = { { 0, 0, 320, 240 }, { 340, 0, 660, 240 }, { 680, 0, 1000, 240 },
	{ 0, 260, 320, 500 }, { 340, 260, 660, 500 }, { 680, 260, 1000, 500 } };

	int nIndex = 0;
	while (nIndex < programNum)
	{
		AVProgram * pProg = g_fmt_ctx->programs[nIndex];
		CProgram* prog = new CProgram;
		prog->Init(pProg->id, g_fmt_ctx, rt[nIndex], hwnd);

		cout << "Program ID: " << pProg->id << endl;
		cout << "Program Num: " << pProg->program_num << endl;
		cout << "nb_streams:  " << pProg->nb_stream_indexes << endl;
		for (unsigned int i = 0; i < pProg->nb_stream_indexes; ++i)
		{
			int streamIndex = pProg->stream_index[i];
			AVMediaType streamType = g_fmt_ctx->streams[streamIndex]->codec->codec_type;
			cout << "	stream index: " << streamIndex << " stream type: " << streamType << endl;
			prog->SetStreamIndex(streamIndex, streamType);
		}
		cout << "pmt_id: " << pProg->pmt_pid << endl;
		cout << "pcr_id: " << pProg->pcr_pid << endl;
		cout << "start_pts: " << pProg->start_time << endl;
		double d = pProg->start_time * av_q2d(G_AV_BASE_TIME_Q);
		cout << "start_time: " << d << endl;
		cout << "end_pts: " << pProg->end_time << endl;
		d = pProg->end_time * av_q2d(G_AV_BASE_TIME_Q);
		cout << "end_time: " << d << endl;

		prog->StartDecoder();
		g_Program.push_back(prog);

		nIndex++;
	}

	//查找流信息
	av_find_best_stream(g_fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	int readNum = 100;
	while (readNum > 0)
	{
		av_read_frame(g_fmt_ctx, &packet);
		cout << packet.stream_index << endl;
		readNum--;
	}


	// 消息循环    
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

__Error:
	if (g_fmt_ctx)
		avformat_close_input(&g_fmt_ctx);

	avformat_network_deinit();

	return 0;
}

// 消息处理函数的实现  
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
