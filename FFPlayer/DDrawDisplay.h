 #pragma once

#include<ddraw.h>//添加头文件
//#include "StreamDecoder.h"
#include "FFDecoder.h"


//加载库支持
#pragma  comment(lib,"ddraw.lib")
#pragma  comment(lib,"dxguid.lib")

#pragma pack( push, 1 )
typedef struct INT_DEC
{
	int inte;		// 整数部分
	int deci;		// 小数部分*64
} INT_DEC;
#pragma pack( pop )


class CDDrawDisplay
{
public:
	CDDrawDisplay(void);
	~CDDrawDisplay(void);

public:
	BOOL InitDDraw(HWND hWnd, int nOffScreenWidth, int nOffScreenHeight);
	void FreeDDraw();//释放对象
	void ClearMemory();//屏幕清除
	void Calc(int sWidth, int sHeight, int dWidth, int dHeight,	// 计算缩放算法，s=源图像宽高，d=目标图像宽高
		const unsigned char *pY, int linesizeY,	// Y
		const unsigned char *pU, int linesizeU,	// U
		const unsigned char *pV, int linesizeV);// V
	// 取得指定坐标点颜色
	BOOL get_pixcel(int x, int y, unsigned char * pY, unsigned char * pU, unsigned char * pV);
	// 取得指定行YUV
	void get_line( unsigned char *lpLine, int line, int width, bool mod );
	//////////////////////////////////////////////////////////////////////////
	BOOL RenderYUV(CRect rect, const FFVideoData* pVFData, const FFVideoInfo* pVFInfo, BOOL bSel);
	//////////////////////////////////////////////////////////////////////////
	inline unsigned char AdjustByte( int tmp ){return (byte)(( tmp>=0 && tmp<=255 ) ? tmp : (tmp<0 ? 0 : 255));};
	BOOL AreOverlaysSupported();//检测设备是否支持Overlay模式
	/////////////////////////////ctx 15-01-04 begin/////////////////////////////////////////////
	void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV);
	BOOL InitVideo(CRect destRect,int R, int G, int B);
	////////////////////////////////ctx 15-01-04 end //////////////////////////////////////////
//     void SetChannel(int channel) {m_channel = channel;}
//     void SetShowType(int type) {m_type = type;}
private:
	LPDIRECTDRAW7 m_pDDraw7;//IDirectDraw7对象
	LPDIRECTDRAWSURFACE7 m_pddsPrimary;//主表面
	LPDIRECTDRAWSURFACE7 m_pddsOverlay;//离屏表面
	LPDIRECTDRAWSURFACE7 m_pddsRect; //画矩形的离屏表面
	DDSURFACEDESC2 m_ddsdOverlay;
	bool m_bOverlay; //是否使用overlay模式

	DDOVERLAYFX     ovfx;
	DDSURFACEDESC2  ddsd;
	DDCAPS          capsDrv;
	DWORD           dwUpdateFlags;

	LPDIRECTDRAWCLIPPER m_lpClipper; //裁剪器
	
	unsigned char *m_pY;	// Y
	unsigned char *m_pU;	// U
	unsigned char *m_pV;	// V
	int m_linesizeY;			// Y 的每行字节数
	int m_linesizeU;			// U 的每行字节数
	int m_linesizeV;			// V 的每行字节数

	///////////////// 临近差值法 //////////////////////////
	//	int m_mapX[1920];			// 最大支持 1920 x 1080 的坐标转换映射
	//	int m_mapY[1080];

	//////////////// 双线性差值法 /////////////////////////
	INT_DEC * m_ptX;		// inte 表示转换映射整数部分
	INT_DEC * m_ptY;		// dec i表示转换映射小数部分(扩大 64 倍取整)

	//////////////// 转CIF双线性插值法 ///////////////////
	INT_DEC * m_pcX;		// inte 表示转换映射整数部分
	INT_DEC * m_pcY;		// dec i表示转换映射小数部分(扩大 64 倍取整)
	unsigned char * m_pquarterY;	// 4分之1（CIF）格式的亮度 Y 数据

	HWND m_hWnd; //保存窗口句柄
	int m_nWidth; //离屏表面显示长度
	int m_nHeight;//离屏表面显示宽度

	int s_width, s_height; //原始图像高度和宽度
	int d_width, d_height;//目标图像高度和宽度

	BOOL m_bSuccessed;//主表面是否创建成功，directx是否初始化成功

};

