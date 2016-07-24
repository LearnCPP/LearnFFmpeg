extern "C"{
#include "libavcodec/dxva2.h"
#include "libavutil/pixfmt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

typedef struct
{
	/* hwaccel context */
	void  *hwaccel_ctx;
	void(*hwaccel_uninit)(AVCodecContext *s);
	int(*hwaccel_get_buffer)(AVCodecContext *s, AVFrame *frame, int flags);
	int(*hwaccel_retrieve_data)(AVCodecContext *s, AVFrame *frame);
	enum AVPixelFormat hwaccel_pix_fmt;
	enum AVPixelFormat hwaccel_retrieved_pix_fmt;
	//video render formate
	D3DFORMAT                    render;
} vlc_va_dxva2_t;

enum AVPixelFormat get_format(AVCodecContext *s, const enum AVPixelFormat *pix_fmts);
int get_buffer(AVCodecContext *s, AVFrame *frame, int flags);
int dxva2_retrieve_data(AVCodecContext *s, AVFrame *frame);