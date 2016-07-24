#include "StdAfx.h"
#include "DDrawDisplay.h"
#include "dxerr9.h"
#include "WinLog.h"

#pragma comment(lib, "dxerr9.lib")
//���Ǳ���YUV���ظ�ʽ
DDPIXELFORMAT ddpfOverlayFormats[] = 
{   
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('U','Y','V','Y'),0,0,0,0,0},  // UYVY
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','U','Y','2'),0,0,0,0,0},  // YUY2
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC,MAKEFOURCC('Y','V','1','2'),0,0,0,0,0}	 //YV12
};

CDDrawDisplay::CDDrawDisplay(void)
	: m_pDDraw7(NULL)
	, m_pddsPrimary(NULL)
	, m_pddsOverlay(NULL)
	, m_pddsRect(NULL)
	, m_lpClipper(NULL)
	, m_hWnd(NULL)
	, m_nWidth(320)
	, m_nHeight(240)
	, m_bSuccessed(FALSE)
	, m_bOverlay(true)
{
	s_width = 0;	s_height = 0;
	d_width = 0;	d_height = 0;

	m_pY = NULL;		m_pU = NULL;		m_pV = NULL;
	m_linesizeY = 0;	m_linesizeU = 0;	m_linesizeV = 0;

	m_ptX = new INT_DEC [1920];
	m_ptY = new INT_DEC [1080];

	m_pcX = new INT_DEC [1920];
	m_pcY = new INT_DEC [1080];
	m_pquarterY = NULL;
}

CDDrawDisplay::~CDDrawDisplay(void)
{

	if( m_pY )	::GlobalFree( m_pY );
	if( m_pU )	::GlobalFree( m_pU );
	if( m_pV )	::GlobalFree( m_pV );

	delete [] m_ptX;
	delete [] m_ptY;

	delete [] m_pcX;
	delete [] m_pcY;

	if( m_pquarterY )	::GlobalFree( m_pquarterY );
}

BOOL CDDrawDisplay::InitDDraw( HWND hWnd, int nOffScreenWidth, int nOffScreenHeight)
{
	m_hWnd = hWnd;
	if( 0 != nOffScreenWidth )//����0ʹ��Ĭ��
		m_nWidth = nOffScreenWidth;
	if( 0 != nOffScreenHeight )
		m_nHeight = nOffScreenHeight;

	HRESULT hRet;
	//��ʼ��IDirectDraw7�ӿ�
	hRet = DirectDrawCreateEx(NULL,(VOID**)&m_pDDraw7,IID_IDirectDraw7,NULL);
	if ( hRet != DD_OK )
	{
		WriteErrorLog(_T("����DirectDraw�ӿ�ʧ��"));
		return FALSE;
	}
	/////////// ������ɫ�� //////////////////////
 	DDSURFACEDESC2 ds;
 	::memset( &ds, 0, sizeof(DDSURFACEDESC2) );
 	ds.dwSize = sizeof(DDSURFACEDESC2);
 	hRet = m_pDDraw7->GetDisplayMode( &ds );
 
 	if( ds.ddpfPixelFormat.dwRGBBitCount != 32 )	// �������32λ���ɫ
 	{
 		hRet = m_pDDraw7->SetDisplayMode( ds.dwWidth,ds.dwHeight,32,0,0);	// �ı���ʾģʽ
 		if( FAILED(hRet) )	return false;
 	}
	/////////////////////////////////////////////

	/*	 * Check for Overlay Support	 */
	if( !AreOverlaysSupported() )	{ 
	//	AfxMessageBox(_T("�����Կ���֧��Overlayģʽ��"));
	//WriteErrorLog(_T("�����Կ���֧��Overlayģʽ��"));
	//	return FALSE;
		m_bOverlay = false;
	}

	//����Э���ȼ���Ϊ����ģʽ
	hRet = m_pDDraw7->SetCooperativeLevel(hWnd,DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES);
	if ( hRet != DD_OK )
	{
		WriteErrorLog(_T("�������ô���ģʽʧ��"));
		return FALSE;
	}

	//��������ṹ��
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS ;//������DirectDraw���湦�ܣ�ddsCaps��Ա��Ч
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	if(FAILED(hRet = m_pDDraw7->CreateSurface(&ddsd,&m_pddsPrimary,NULL)))
	{
		return FALSE;
	}

	//�����ü���
	if(FAILED(m_pDDraw7->CreateClipper(0,&m_lpClipper,NULL)))
	{
		return FALSE;
	}
	if(FAILED(m_lpClipper->SetHWnd(0,hWnd)))
	{
		return FALSE;
	}
	if (FAILED(m_pddsPrimary->SetClipper(m_lpClipper)))
	{
		return FALSE;
	}
	//���������ο���������
	//ZeroMemory(&ddsd,sizeof(DDSURFACEDESC2) );
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize = sizeof(ddsd);
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.dwWidth = m_nWidth;
	ddsd.dwHeight = m_nHeight;
	hRet = m_pDDraw7->CreateSurface(&ddsd,&m_pddsRect,NULL);
	if( FAILED(hRet) )
		return FALSE;

	//�������Ǳ���
	ZeroMemory(&m_ddsdOverlay,sizeof(DDSURFACEDESC2));
	m_ddsdOverlay.dwSize = sizeof(m_ddsdOverlay);
	m_ddsdOverlay.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	if (m_bOverlay)
	{
		m_ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;//DDSCAPS_OFFSCREENPLAIN/* | DDSCAPS_VIDEOMEMORY*/;
	}
	else
	{
		m_ddsdOverlay.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	}
	m_ddsdOverlay.dwWidth = m_nWidth;
	m_ddsdOverlay.dwHeight = m_nHeight;

	//////////////////////////////////////////////////////////////////////////
// 	ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
// 	ddsd.dwSize = sizeof(DDSURFACEDESC2);
// 	ddsd.dwFlags = DDSD_ALL;
// 	hRet = m_pddsPrimary->GetSurfaceDesc(&ddsd);

	//////////////////////////////////////////////////////////////////////////
	DWORD i = 0;
	do 
	{
		m_ddsdOverlay.ddpfPixelFormat = ddpfOverlayFormats[i];
		hRet = m_pDDraw7->CreateSurface( &m_ddsdOverlay, &m_pddsOverlay, NULL );

	} while( FAILED( hRet ) && (++i < 3) );
	if (FAILED(hRet))
	{
//		WriteErrorLog(_T("���ظ�ʽ��֧�֣�"));
		CString strerr;
		strerr.Format(_T("Error: %s error description: %s"), DXGetErrorString9(hRet),
			DXGetErrorDescription9(hRet));
		WriteErrorLog(strerr);
		return FALSE;
	}

	m_bSuccessed = TRUE;

	return TRUE;
}

void CDDrawDisplay::FreeDDraw()
{
	if ( m_pDDraw7 != NULL)
	{
		if (m_pddsPrimary != NULL)
		{//�ͷ�������
			m_pddsPrimary->Release();
			m_pddsPrimary=NULL;
		}

		if (m_pddsOverlay!=NULL)
		{//�ͷ���������
			m_pddsOverlay->Release();
			m_pddsOverlay=NULL;
		}

		if ( m_pddsRect != NULL)
		{
			m_pddsRect->Release();
			m_pddsRect=NULL;
		}

		if (m_lpClipper != NULL )
		{//�ͷż��а�
			m_lpClipper->Release();
			m_lpClipper = NULL;
		}

		//�ͷ�IDirectDraw7����
		m_pDDraw7->Release();
		m_pDDraw7 = NULL;
	}
}

BOOL CDDrawDisplay::AreOverlaysSupported()
{
	DDCAPS capsDrv;

	ZeroMemory(&capsDrv, sizeof(capsDrv));
	capsDrv.dwSize = sizeof(capsDrv);

	HRESULT hr = m_pDDraw7->GetCaps(&capsDrv, NULL);
	if (FAILED( hr ))        return false;

	if (!(capsDrv.dwCaps & DDCAPS_OVERLAY))    return false;
	return true;
}

void CDDrawDisplay::ClearMemory()
{
	if( NULL==m_pDDraw7  || NULL==m_pddsPrimary || NULL==m_pddsOverlay ) 
		return;
	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);

	HRESULT hr = m_pddsOverlay->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT,NULL);
	if( FAILED(hr)) 
		return;
	LPBYTE lpRowStart = (LPBYTE)ddsd.lpSurface;
	for (int i=0; i<m_nHeight; ++i)
	{
		unsigned char  * lpLine = lpRowStart + i*ddsd.lPitch;
		for (int j=0; j<m_nWidth; ++j)
		{
			*lpLine++=128;
			*lpLine++=0;
		}
	}
	m_pddsOverlay->Unlock(NULL);
}

void CDDrawDisplay::Calc(int sWidth, int sHeight, int dWidth, int dHeight,	// ���������㷨��sW,sH=Դͼ���ߣ�dW,dH=Ŀ��ͼ����
		const unsigned char *pY, int linesizeY,		// Y
		const unsigned char *pU, int linesizeU,		// U
		const unsigned char *pV, int linesizeV)		// V
{
	if( s_width != sWidth || s_height != sHeight || d_width != dWidth || d_height != dHeight ||	// �������ߴ��б仯
		m_linesizeY != linesizeY || m_linesizeU != linesizeU || m_linesizeV != linesizeV )
	{
		s_width = sWidth;	s_height = sHeight;
		d_width = dWidth;	d_height = dHeight;

		double xc = (double)d_width / s_width;		// �任����
		double yc = (double)d_height/ s_height;

		int i;
		///////////////////// �ٽ����ز�ֵ�� ///////////////////
//		for( i=0; i<d_height; i++ )
//		{
//			m_mapY[i] = (int)(i / yc);
//			if( m_mapY[i] >= s_height )	m_mapY[i] = s_height-1;
//		}
//		for( i=0; i<d_width; i++ )
//		{
//			m_mapX[i] = (int)(i / xc);
//			if( m_mapX[i] >= s_width )	m_mapX[i] = s_width-1;
//		}
		///////////////////// ˫���Բ�ֵ�� ////////////////////
		for( i=0; i<d_height; i++ )
		{
			double p = i/yc;
			m_ptY[i].inte = (int)p;
			m_ptY[i].deci = (int)( (p-(int)p) * 64 );
			if( m_ptY[i].inte >= s_height )
			{
				m_ptY[i].inte = s_height-1;
				m_ptY[i].deci = 0;
			}
		}
		for( i=0; i<d_width; i++ )
		{
			double p = i/xc;
			m_ptX[i].inte = (int)p;
			m_ptX[i].deci = (int)( (p-(int)p) * 64 );
			if( m_ptX[i].inte >= s_width )
			{
				m_ptX[i].inte = s_width-1;
				m_ptX[i].deci = 0;
			}
		}
		//////////////////////////////////////////////////////
		m_linesizeY = linesizeY;
		m_linesizeU = linesizeU;
		m_linesizeV = linesizeV;

		if( m_pY ){	::GlobalFree( m_pY );	m_pY = NULL;	}
		if( m_pU ){	::GlobalFree( m_pU );	m_pU = NULL;	}
		if( m_pV ){	::GlobalFree( m_pV );	m_pV = NULL;	}

		m_pY = (unsigned char *)::GlobalAlloc( GPTR, linesizeY * s_height );
		m_pU = (unsigned char *)::GlobalAlloc( GPTR, linesizeU * s_height / 2 );
		m_pV = (unsigned char *)::GlobalAlloc( GPTR, linesizeV * s_height / 2 );

		if( s_width >= 640 && s_height >= 480 )	// ����Դ�� D1 ����
		{										// ת��Ϊ CIF �ֱ���
			double xc = (double)d_width / (s_width/2);		// �任����
			double yc = (double)d_height/ (s_height/2);

			for( i=0; i<d_height; i++ )
			{
				double p = i/yc;
				m_pcY[i].inte = (int)p;
				m_pcY[i].deci = (int)( (p-(int)p) * 64 );
				if( m_pcY[i].inte >= s_height/2 )
				{
					m_pcY[i].inte = s_height/2-1;
					m_pcY[i].deci = 0;
				}
			}
			for( i=0; i<d_width; i++ )
			{
				double p = i/xc;
				m_pcX[i].inte = (int)p;
				m_pcX[i].deci = (int)( (p-(int)p) * 64 );
				if( m_pcX[i].inte >= s_width/2 )
				{
					m_pcX[i].inte = s_width/2-1;
					m_pcX[i].deci = 0;
				}
			}
			if( m_pquarterY )	::GlobalFree( m_pquarterY );
			m_pquarterY = (unsigned char *)::GlobalAlloc( GPTR, s_width/2 * s_height/2 );
		}
		else		// �� D1 �ֱ���
		{
			if( m_pquarterY )	::GlobalFree( m_pquarterY );
			m_pquarterY = NULL;
		}
	}
	::memcpy( m_pY, pY, linesizeY * s_height );
	::memcpy( m_pU, pU, linesizeU * s_height/2 );
	::memcpy( m_pV, pV, linesizeV * s_height/2 );

	if( m_pquarterY )
	{
		unsigned char *p = m_pquarterY;
/*		for( int i=0; i<s_height/2; i++ )
		for( int j=0; j<s_width/2; j++ )
		{
			*p = (	*(m_pY + linesizeY * i*2 + j*2 )	+				// 4 ������ƽ��Ϊ1������
					*(m_pY + linesizeY * i*2 + j*2 + 1 )	+
					*(m_pY + linesizeY *(i*2+1) + j*2 ) +
					*(m_pY + linesizeY *(i*2+1) + j*2 + 1 )
				)/4;
			p++;
		}
*/
		/////////////// ����������Ż� ////////////////
		for( int i=0; i<s_height/2; i++ )
		{
			unsigned char *pY1 = (unsigned char *)pY + linesizeY * i*2;			// ż���е�ַ
			unsigned char *pY2 = (unsigned char *)pY + linesizeY * (i*2 +1 );	// �����е�ַ
			for( int j=0; j<s_width/2; j++ )
			{
				unsigned v = *pY1++;
				v += *pY1++;
				v += *pY2++;
				v += *pY2++;

				*p ++= v>>2;
			}
		}
	}
}

// ȡ��ָ���������ɫ
BOOL CDDrawDisplay::get_pixcel(int x, int y, unsigned char * pY, unsigned char * pU, unsigned char * pV)
{
	ASSERT( x>=0 && y >= 0 );
	if( x >= s_width || y >= s_height )		
		return FALSE;

	/////////////// �ٽ����ز�ֵ�� ///////////////////
	//	x = m_mapX[ x ];	y = m_mapY[ y ];

	//	*pY = *( m_pY + m_linesizeY * y + x );				// һ�� Y ��ʾ 1�� ��
	//	*pU = *( m_pU + m_linesizeU * (y/2) + (x/2) );		// һ�� U ��ʾ 4�� ��
	//	*pV = *( m_pV + m_linesizeV * (y/2) + (x/2) );		// һ�� V ��ʾ 4�� ��

	/////////////////// ˫���Բ�ֵ�� /////////////////////
	//	�������ص�P(x+x',y+y')��ֵ��ͨ���ٽ�4�����ֵ����õ�
	//	A�����������������֣�A'��ʾ����������С������
	//	��ֵ���㹫ʽΪ��
	//	p(x+x',y+y') = (1-x')(1-y')p(x,y) + (1-x')y'p(x,y+1) + x'(1-y')p(x+1,y) + x'y'p(x+1,y+1)
	//////////////////////////////////////////////////////
	if( s_width >= 640 && s_height >= 480 && d_width <= 640 && d_height <= 480 )		// ����D1תCIF���㷨
	{
		int a = m_pcX[ x ].deci;				// С����������
		int b = m_pcY[ y ].deci;
		x = m_pcX[ x ].inte;					// ��������
		y = m_pcY[ y ].inte;

		unsigned char *ptY = m_pquarterY + s_width/2 * y + x;			// ��׼���ַ
		unsigned char *ptU = m_pU + m_linesizeU *y + x;
		unsigned char *ptV = m_pV + m_linesizeV *y + x;

		if( y+1 >= s_height/2 || x+1 >= s_width/2 )		// Խ��
		{
			*pY = *ptY;		*pU = *ptU;		*pV = *ptV;
			return true;
		}

		int _a_b = (64-a)*(64-b);
		int _ab  = (64-a)*b;
		int a_b  = a*(64-b);
		int ab   = a*b;
		// �ٽ�4���������Ӧ�� YUV
		unsigned char y00, y01, y10, y11;							// (x+0,y+0) (x+1,y+0)
		unsigned char u00, u01, u10, u11;							// (x+0,y+1) (x+1,y+1)
		unsigned char v00, v01, v10, v11;

		y00 = *ptY;	u00 = *ptU;	v00 = *ptV;

		y01 = *( ptY + s_width/2 );
		y10 = *( ptY + 1 );
		y11 = *( ptY + s_width/2 + 1 );

		if( 0 == x%2 && 0 == y%2 )	// Xż��Yż
		{
			int yy = _a_b*y00 + _ab*y01 + a_b*y10 + ab*y11;
			*pY = AdjustByte(yy/4096);

			*pU = u00;	*pV = v00;	return true;
		}

		if( 0 == x%2 && 1 == y%2 )	// Xż��Y��
		{
			u01 = *( ptU + m_linesizeU );
			u10 = u00;
			u11 = u01;

			v01 = *( ptV + m_linesizeV );
			v10 = v00;
			v11 = v01;
		}
		else if( 1 == x%2 && 0 == y%2 )	// X�棬Yż
		{
			u01 = u00;
			u10 = *( ptU + 1 );
			u11 = u10;

			v01 = v00;
			v10 = *( ptV + 1 );
			v11 = v10;
		}
		else if( 1 == x%2 && 1 == y%2 )	// X�棬Y��
		{
			u01 = *( ptU + m_linesizeU );
			u10 = *( ptU + 1 );
			u11 = *( ptU + m_linesizeU + 1 );

			v01 = *( ptV + m_linesizeV );
			v10 = *( ptV + 1 );
			v11 = *( ptV + m_linesizeV + 1 );
		}

		int yy = _a_b*y00 + _ab*y01 + a_b*y10 + ab*y11;
		int uu = _a_b*u00 + _ab*u01 + a_b*u10 + ab*u11;
		int vv = _a_b*v00 + _ab*v01 + a_b*v10 + ab*v11;

		*pY = AdjustByte(yy/4096);
		*pU = AdjustByte(uu/4096);
		*pV = AdjustByte(vv/4096);

		return true;
	}
	int a = m_ptX[ x ].deci;				// С����������
	int b = m_ptY[ y ].deci;
	x = m_ptX[ x ].inte;					// ��������
	y = m_ptY[ y ].inte;

	unsigned char *ptY = m_pY + m_linesizeY * y + x;			// ��׼���ַ
	unsigned char *ptU = m_pU + m_linesizeU *(y/2) + (x/2);
	unsigned char *ptV = m_pV + m_linesizeV *(y/2) + (x/2);

	if( y+1 >= s_height || x+1 >= s_width )		// Խ��
	{
		*pY = *ptY;		*pU = *ptU;		*pV = *ptV;
		return true;
	}

	int _a_b = (64-a)*(64-b);
	int _ab  = (64-a)*b;
	int a_b  = a*(64-b);
	int ab   = a*b;
	// �ٽ�4���������Ӧ�� YUV
	unsigned char y00, y01, y10, y11;							// (x+0,y+0) (x+1,y+0)
	unsigned char u00, u01, u10, u11;							// (x+0,y+1) (x+1,y+1)
	unsigned char v00, v01, v10, v11;

	y00 = *ptY;	u00 = *ptU;	v00 = *ptV;

	y01 = *( ptY + m_linesizeY );
	y10 = *( ptY + 1 );
	y11 = *( ptY + m_linesizeY + 1 );

	if( 0 == x%2 && 0 == y%2 )	// Xż��Yż
	{
		int yy = _a_b*y00 + _ab*y01 + a_b*y10 + ab*y11;
		*pY = AdjustByte(yy/4096);

		*pU = u00;	*pV = v00;	return true;
	}

	if( 0 == x%2 && 1 == y%2 )	// Xż��Y��
	{
		u01 = *( ptU + m_linesizeU );
		u10 = u00;
		u11 = u01;

		v01 = *( ptV + m_linesizeV );
		v10 = v00;
		v11 = v01;
	}
	else if( 1 == x%2 && 0 == y%2 )	// X�棬Yż
	{
		u01 = u00;
		u10 = *( ptU + 1 );
		u11 = u10;

		v01 = v00;
		v10 = *( ptV + 1 );
		v11 = v10;
	}
	else if( 1 == x%2 && 1 == y%2 )	// X�棬Y��
	{
		u01 = *( ptU + m_linesizeU );
		u10 = *( ptU + 1 );
		u11 = *( ptU + m_linesizeU + 1 );

		v01 = *( ptV + m_linesizeV );
		v10 = *( ptV + 1 );
		v11 = *( ptV + m_linesizeV + 1 );
	}

	int yy = _a_b*y00 + _ab*y01 + a_b*y10 + ab*y11;
	int uu = _a_b*u00 + _ab*u01 + a_b*u10 + ab*u11;
	int vv = _a_b*v00 + _ab*v01 + a_b*v10 + ab*v11;

	*pY = AdjustByte(yy/4096);
	*pU = AdjustByte(uu/4096);
	*pV = AdjustByte(vv/4096);

	return true;
}

// ȡ��ָ����YUV
void CDDrawDisplay::get_line( unsigned char *lpLine, int line, int width, bool mod )
{
	BOOL bD1;
	if( s_width >= 640 && s_height >= 480 && d_width <= 640 && d_height <= 480 )		// ����D1תCIF���㷨
		bD1 = TRUE;
	else	
		bD1 = FALSE;

	for( int i=0; i<width; i++, mod = !mod )
	{
		int a, b;						// С����������
		int x, y;						// ��������
		unsigned char *ptY, *ptU, *ptV;	// ��׼���ַ
		bool bOver=  FALSE;				// Խ��
		int linesizeY;

		if( bD1 )
		{
			a = m_pcX[ i ].deci;				
			b = m_pcY[ line ].deci;
			x = m_pcX[ i ].inte;				// ��������
			y = m_pcY[ line ].inte;

			linesizeY = s_width/2;
			ptY = m_pquarterY + linesizeY * y + x;
			ptU = m_pU + m_linesizeU * y + x;
			ptV = m_pV + m_linesizeV * y + x;

			if( y+1 >= s_height/2 || x+1 >= s_width/2 )		// Խ��
				bOver = TRUE;
		}
		else
		{
			a = m_ptX[ i ].deci;				
			b = m_ptY[ line ].deci;
			x = m_ptX[ i ].inte;				
			y = m_ptY[ line ].inte;

			linesizeY = m_linesizeY;
			ptY = m_pY + linesizeY * y + x;			
			ptU = m_pU + m_linesizeU *(y/2) + (x/2);
			ptV = m_pV + m_linesizeV *(y/2) + (x/2);

			if( y+1 >= s_height || x+1 >= s_width )		// Խ��
				bOver = TRUE;
		}

		if( bOver )
		{
			if( !mod )	
				*lpLine++ = *ptV;
			else		
				*lpLine++ = *ptU;
			*lpLine++ = *ptY;		
				continue;
		}

		int	_a_b	=	(64-a)*(64-b);
		int _ab		=	(64-a)*b;
		int a_b		=	a*(64-b);
		int ab		=	a*b;

		if( !(x%2) && !(y%2) )		// Xż��Yż
		{
			if( !mod )	*lpLine++ = *ptU;
			else		*lpLine++ = *ptV;
		}
		else if( !(x%2) && (y%2) )	// Xż��Y��
		{
			if( !mod )
				*lpLine++ = AdjustByte( ((64-b)*ptU[0] + b*ptU[m_linesizeU])/64 );
			else	*lpLine++ = AdjustByte( ((64-b)*ptV[0] + b*ptV[m_linesizeV])/64 );
		}
		else if( (x%2) && !(y%2) )	// X�棬Yż
		{
			if( !mod )
				*lpLine++ = AdjustByte( ((64-a)*ptU[0] + a*ptU[1])/64 );
			else	*lpLine++ = AdjustByte( ((64-a)*ptV[0] + a*ptV[1])/64 );
		}
		else if( x%2 && y%2 )	// X�棬Y��
		{
			if( !mod )
				*lpLine++ = AdjustByte((_a_b*ptU[0] + _ab*ptU[m_linesizeU] + a_b*ptU[1] + ab*ptU[m_linesizeU+1])/4096);
			else	*lpLine++ = AdjustByte((_a_b*ptV[0] + _ab*ptV[m_linesizeV] + a_b*ptV[1] + ab*ptV[m_linesizeV+1])/4096);
		}
		*lpLine++ = AdjustByte(( _a_b*ptY[0] + _ab*ptY[linesizeY] + a_b*ptY[1] + ab*ptY[linesizeY+1] )/4096);
	}
}

BOOL CDDrawDisplay::RenderYUV( CRect rect,const FFVideoData* pVFData, const FFVideoInfo* pVFInfo, BOOL bSel )
{
	if( !m_bSuccessed ) //��ʼ��DirectX�����񲻻�
	{
		//AppLog(_T("DirectX��ʼ��ʧ�ܣ���鿴������־������ĵط���"));
		return FALSE;
	}
//	ClearMemory();

	Calc(pVFInfo->width,pVFInfo->height,m_nWidth,m_nHeight,
		pVFData->pY,pVFData->linesizey,
		pVFData->pU,pVFData->linesizeu,
		pVFData->pV,pVFData->linesizev);

	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);

 	HRESULT hr = m_pddsOverlay->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT
 														 ,NULL);
 	if( hr== DDERR_SURFACELOST )// return FALSE;
 	{
 		hr = m_pddsOverlay->Restore();
 		hr = m_pddsOverlay->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT
 			,NULL);
 	}
 	if( FAILED(hr) )
 		return FALSE;
 
 	LPBYTE  lpRowStart = (LPBYTE )ddsd.lpSurface;
 	for ( int i=0; i<m_nHeight; i++ )
 	{
 		unsigned char * lpLine = lpRowStart + ( 0 * ddsd.lPitch + 0 * 2 ) + i * ddsd.lPitch;
 		get_line( lpLine, i, m_nWidth, 0%2?true:false );
 	}
 	m_pddsOverlay->Unlock(NULL);


	if ( bSel ) //ѡ�е�ǰ���ο�
	{//ѡ�м�����Ƶ���Ȱ�YUV����Blt�����������棬Ȼ����Blt��������
		CRect rt(0,0,m_nWidth,m_nHeight);
		hr = m_pddsRect->Blt(rt,m_pddsOverlay,rt,DDBLT_WAIT,NULL);

		if ( FAILED(hr) )	
		{
			CString strerr;
			strerr.Format(_T("Error: %s error description: %s"), DXGetErrorString9(hr),
				DXGetErrorDescription9(hr));
			WriteErrorLog(strerr);
			return FALSE;
		}

		HDC hdc = NULL;
		hr = m_pddsRect->GetDC(&hdc);
		if( SUCCEEDED(hr)&& hdc)
		{
			HPEN pen = CreatePen(PS_SOLID,10, RGB(0,255,0) );
			HGDIOBJ pOldpen = ::SelectObject(hdc,pen );

			::MoveToEx(hdc, 0, 0,NULL );
			LineTo( hdc,m_nWidth, 0 );
			LineTo( hdc,m_nWidth, m_nHeight );
			LineTo( hdc, 0, m_nHeight );
			LineTo( hdc, 0, 0 );

			::SelectObject(hdc,pOldpen);
			m_pddsRect->ReleaseDC(hdc);
			::DeleteObject( pen );
		}
		hr = m_pddsPrimary->Blt(&rect,m_pddsRect,CRect(0,0,m_nWidth,m_nHeight),DDBLT_WAIT,NULL);
	}
	else
	{//û��ѡ������£�YUV����ֱ��Blt��������
		hr = m_pddsPrimary->Blt(&rect,m_pddsOverlay,CRect(0,0,m_nWidth,m_nHeight),DDBLT_WAIT,NULL);
	}
	

	if( FAILED(hr) )
	{
		if (hr == DDERR_SURFACELOST)
				m_pddsPrimary->Restore();
			else
			{
				CString strErr;
				strErr.Format(_T("Blt error, return value=%d"), MAKE_DDHRESULT(hr));
				WriteErrorLog(strErr);
				return FALSE;
			}
	}
	

	return TRUE;
}

////////////////////////////////ctx 15-01-04 add begin//////////////////////////////////////////
void CDDrawDisplay::RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV)
{
	*pY = AdjustByte( 16 + ( 66*R + 129*G + 25*B )/256 );
	*pU = AdjustByte( 128 + ( -38*R - 74*G + 112*B )/256 );
	*pV = AdjustByte( 128 + ( 112*R - 94*G - 18*B )/256 );
}

BOOL CDDrawDisplay::InitVideo(CRect destRect,int R, int G, int B)
{
	ZeroMemory(&ddsd, sizeof(ddsd)); 
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	HRESULT hr = m_pddsOverlay->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,NULL);
	if( FAILED(hr) ) return FALSE;
	LPBYTE  lpRowStart = (LPBYTE )ddsd.lpSurface;
	unsigned char Y,U,V;
	
	for ( int i=0; i<m_nHeight; ++i)
	{
		LPBYTE lpLine = lpRowStart + i*ddsd.lPitch;
		RGB2YUV((unsigned char)R,(unsigned char)G,(unsigned char)B,&Y,&U,&V);
		for (int j=0; j<m_nWidth; ++j)
		{
			if (j % 2== 0)
				*lpLine++=U;
			else
				*lpLine++=V;

			*lpLine++=Y;	
		}
	}
	m_pddsOverlay->Unlock(NULL);

	hr = m_pddsPrimary->Blt(&destRect,m_pddsOverlay,CRect(0,0,m_nWidth,m_nHeight),DDBLT_WAIT,NULL);
	if (FAILED(hr))
		return FALSE;

	return TRUE;
}
///////////////////////////////ctx 15-01-04 end ///////////////////////////////////////////
