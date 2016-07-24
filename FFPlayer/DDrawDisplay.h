 #pragma once

#include<ddraw.h>//���ͷ�ļ�
//#include "StreamDecoder.h"
#include "FFDecoder.h"


//���ؿ�֧��
#pragma  comment(lib,"ddraw.lib")
#pragma  comment(lib,"dxguid.lib")

#pragma pack( push, 1 )
typedef struct INT_DEC
{
	int inte;		// ��������
	int deci;		// С������*64
} INT_DEC;
#pragma pack( pop )


class CDDrawDisplay
{
public:
	CDDrawDisplay(void);
	~CDDrawDisplay(void);

public:
	BOOL InitDDraw(HWND hWnd, int nOffScreenWidth, int nOffScreenHeight);
	void FreeDDraw();//�ͷŶ���
	void ClearMemory();//��Ļ���
	void Calc(int sWidth, int sHeight, int dWidth, int dHeight,	// ���������㷨��s=Դͼ���ߣ�d=Ŀ��ͼ����
		const unsigned char *pY, int linesizeY,	// Y
		const unsigned char *pU, int linesizeU,	// U
		const unsigned char *pV, int linesizeV);// V
	// ȡ��ָ���������ɫ
	BOOL get_pixcel(int x, int y, unsigned char * pY, unsigned char * pU, unsigned char * pV);
	// ȡ��ָ����YUV
	void get_line( unsigned char *lpLine, int line, int width, bool mod );
	//////////////////////////////////////////////////////////////////////////
	BOOL RenderYUV(CRect rect, const FFVideoData* pVFData, const FFVideoInfo* pVFInfo, BOOL bSel);
	//////////////////////////////////////////////////////////////////////////
	inline unsigned char AdjustByte( int tmp ){return (byte)(( tmp>=0 && tmp<=255 ) ? tmp : (tmp<0 ? 0 : 255));};
	BOOL AreOverlaysSupported();//����豸�Ƿ�֧��Overlayģʽ
	/////////////////////////////ctx 15-01-04 begin/////////////////////////////////////////////
	void RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV);
	BOOL InitVideo(CRect destRect,int R, int G, int B);
	////////////////////////////////ctx 15-01-04 end //////////////////////////////////////////
//     void SetChannel(int channel) {m_channel = channel;}
//     void SetShowType(int type) {m_type = type;}
private:
	LPDIRECTDRAW7 m_pDDraw7;//IDirectDraw7����
	LPDIRECTDRAWSURFACE7 m_pddsPrimary;//������
	LPDIRECTDRAWSURFACE7 m_pddsOverlay;//��������
	LPDIRECTDRAWSURFACE7 m_pddsRect; //�����ε���������
	DDSURFACEDESC2 m_ddsdOverlay;
	bool m_bOverlay; //�Ƿ�ʹ��overlayģʽ

	DDOVERLAYFX     ovfx;
	DDSURFACEDESC2  ddsd;
	DDCAPS          capsDrv;
	DWORD           dwUpdateFlags;

	LPDIRECTDRAWCLIPPER m_lpClipper; //�ü���
	
	unsigned char *m_pY;	// Y
	unsigned char *m_pU;	// U
	unsigned char *m_pV;	// V
	int m_linesizeY;			// Y ��ÿ���ֽ���
	int m_linesizeU;			// U ��ÿ���ֽ���
	int m_linesizeV;			// V ��ÿ���ֽ���

	///////////////// �ٽ���ֵ�� //////////////////////////
	//	int m_mapX[1920];			// ���֧�� 1920 x 1080 ������ת��ӳ��
	//	int m_mapY[1080];

	//////////////// ˫���Բ�ֵ�� /////////////////////////
	INT_DEC * m_ptX;		// inte ��ʾת��ӳ����������
	INT_DEC * m_ptY;		// dec i��ʾת��ӳ��С������(���� 64 ��ȡ��)

	//////////////// תCIF˫���Բ�ֵ�� ///////////////////
	INT_DEC * m_pcX;		// inte ��ʾת��ӳ����������
	INT_DEC * m_pcY;		// dec i��ʾת��ӳ��С������(���� 64 ��ȡ��)
	unsigned char * m_pquarterY;	// 4��֮1��CIF����ʽ������ Y ����

	HWND m_hWnd; //���洰�ھ��
	int m_nWidth; //����������ʾ����
	int m_nHeight;//����������ʾ���

	int s_width, s_height; //ԭʼͼ��߶ȺͿ��
	int d_width, d_height;//Ŀ��ͼ��߶ȺͿ��

	BOOL m_bSuccessed;//�������Ƿ񴴽��ɹ���directx�Ƿ��ʼ���ɹ�

};

