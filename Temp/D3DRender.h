#pragma once
#include<d3d9.h>
#include <d3dx9.h>
#include <windows.h>
extern "C"{
#include<libavformat/avformat.h>
}

#define FOURCC_PIX_FMT_YUV420P MAKEFOURCC('I','4','2','0') /* 16  YVU420 planar */
#define FOURCC_PIX_FMT_IYUV    MAKEFOURCC('I','Y','U','V') /* 16  IYUV   planar */
#define FOURCC_PIX_FMT_YVU410  MAKEFOURCC('Y','V','U','9') /*  9  YVU 4:1:0     */
#define FOURCC_PIX_FMT_YVU420  MAKEFOURCC('Y','V','1','2') /* 12  YVU 4:2:0     */
#define FOURCC_PIX_FMT_YUY2    MAKEFOURCC('Y','U','Y','2') /* 16  YUYV 4:2:2 */
#define FOURCC_PIX_FMT_UYVY    MAKEFOURCC('U','Y','V','Y') /* 16  UYVY 4:2:2 */
#define FOURCC_PIX_FMT_YVYU    MAKEFOURCC('Y','V','Y','U') /* 16  UYVY 4:2:2 */

#define FOURCC_PIX_FMT_RGB332  MAKEFOURCC('R','G','B','1') /*  8  RGB-3-3-2     */
#define FOURCC_PIX_FMT_RGB555  MAKEFOURCC('R','G','B','O') /* 16  RGB-5-5-5     */
#define FOURCC_PIX_FMT_RGB565  MAKEFOURCC('R','G','B','P') /* 16  RGB-5-6-5     */
#define FOURCC_PIX_FMT_RGB555X MAKEFOURCC('R','G','B','Q') /* 16  RGB-5-5-5 BE  */
#define FOURCC_PIX_FMT_RGB565X MAKEFOURCC('R','G','B','R') /* 16  RGB-5-6-5 BE  */
#define FOURCC_PIX_FMT_BGR15   MAKEFOURCC('B','G','R',15)  /* 15  BGR-5-5-5     */
#define FOURCC_PIX_FMT_RGB15   MAKEFOURCC('R','G','B',15)  /* 15  RGB-5-5-5     */
#define FOURCC_PIX_FMT_BGR16   MAKEFOURCC('B','G','R',16)  /* 32  BGR-5-6-5     */
#define FOURCC_PIX_FMT_RGB16   MAKEFOURCC('R','G','B',16)  /* 32  RGB-5-6-5     */
#define FOURCC_PIX_FMT_BGR24   MAKEFOURCC('B','G','R',24)  /* 24  BGR-8-8-8     */
#define FOURCC_PIX_FMT_RGB24   MAKEFOURCC('R','G','B',24)  /* 24  RGB-8-8-8     */
#define FOURCC_PIX_FMT_BGR32   MAKEFOURCC('B','G','R',32)  /* 32  BGR-8-8-8-8   */
#define FOURCC_PIX_FMT_RGB32   MAKEFOURCC('R','G','B',32)  /* 32  RGB-8-8-8-8   */
#define FOURCC_PIX_FMT_BGR     (('B'<<24)|('G'<<16)|('R'<<8))
#define FOURCC_PIX_FMT_BGR8    (FOURCC_PIX_FMT_BGR|8)

typedef struct {
	unsigned int  mplayer_fmt;   /**< Given by MPlayer */
	D3DFORMAT     fourcc;        /**< Required by D3D's test function */
} struct_fmt_table;

typedef enum back_buffer_action {
	BACKBUFFER_CREATE,
	BACKBUFFER_RESET
} back_buffer_action_e;

#define DISPLAY_FORMAT_TABLE_ENTRIES (sizeof(fmt_table) / sizeof(fmt_table[0]))

class CD3DRender
{
public:
	CD3DRender();
	~CD3DRender();


public:
	/* 初始render.	*/
	virtual bool init_render(void* ctx, int w, int h, int pix_fmt);

	/* 渲染一帧.	*/
	virtual bool render_one_frame(AVFrame* data/*, AVFrame* info, int pix_fmt*/, const RECT& rect, bool bSel);

	/* 调整大小.	*/
	virtual void re_size(int width, int height);

	/* 设置宽高比.	*/
	virtual void aspect_ratio(int srcw, int srch, bool enable_aspect);

	/* 撤销render.		*/
	virtual void destory_render();

private:
	void fill_d3d_presentparams(D3DPRESENT_PARAMETERS *present_params);
	bool configure_d3d();
	void destroy_d3d_surfaces();
	bool change_d3d_backbuffer(back_buffer_action_e action);
	bool create_d3d_surfaces();
	bool reconfigure_d3d();
	int query_format(uint32_t movie_fmt);

private:
	LPDIRECT3D9 m_d3d_handle;                 /**< Direct3D Handle */
	LPDIRECT3DDEVICE9 m_d3d_device;           /**< The Direct3D Adapter */
	LPDIRECT3DSURFACE9 m_d3d_surface;         /**< Offscreen Direct3D Surface. */
	LPDIRECT3DTEXTURE9 m_d3d_texture_osd;     /**< Direct3D Texture. Uses RGBA */
	LPDIRECT3DTEXTURE9 m_d3d_texture_system;  /**< Direct3D Texture. System memory cannot lock a normal texture. Uses RGBA */
	LPDIRECT3DSURFACE9 m_d3d_backbuf;         /**< Video card's back buffer (used to display next frame) */
	D3DLOCKED_RECT m_locked_rect;             /**< The locked offscreen surface */
	D3DFORMAT m_desktop_fmt;                  /**< Desktop (screen) colorspace format. */
	D3DFORMAT m_movie_src_fmt;                /**< Movie colorspace format (depends on the movie's codec) */
	D3DPRESENT_PARAMETERS m_present_params;
	RECT m_last_client_rect;

	LPD3DXLINE              m_pLine; //Direct3D线对象 
//	D3DXVECTOR2*            m_pLineArr; //线段顶点

	int m_cur_backbuf_width;         /**< Current backbuffer width */
	int m_cur_backbuf_height;        /**< Current backbuffer height */
	int m_device_caps_power2_only;   /**< 1 = texture sizes have to be power 2
									 0 = texture sizes can be anything */
	int m_device_caps_square_only;   /**< 1 = textures have to be square
									 0 = textures do not have to be square */
	int m_device_texture_sys;        /**< 1 = device can texture from system memory
									 0 = device requires shadow */
	int m_max_texture_width;         /**< from the device capabilities */
	int m_max_texture_height;        /**< from the device capabilities */

	int m_osd_width;                 /**< current width of the OSD */
	int m_osd_height;                /**< current height of the OSD */
	int m_osd_texture_width;         /**< current width of the OSD texture */
	int m_osd_texture_height;        /**< current height of the OSD texture */

	HWND m_hwnd;
	int m_image_width;
	int m_image_height;

	/* 保持宽高.	*/
	bool m_keep_aspect;

	/* 宽高比.		*/
	float m_window_aspect;
};

