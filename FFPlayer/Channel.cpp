#include "StdAfx.h"
#include "Channel.h"
#include "WinLog.h"

CChannel::CChannel(HWND hWnd/*, int id*/)
	:m_hWnd(hWnd)
	/*,m_nID(id)*/
	, m_pD3DRender(NULL)
	/*, m_pQueue(NULL)*/
	, m_pDecoder(NULL)
	, m_cColorBar(0)
	, m_nColorBarPixcel(0)
	, m_nSeparatorPixcel(0)	
	, m_nColorBarWidth(0)
	, m_nSeparatorWidth(0)
	, m_nVal_l(0)
	, m_nVal_r(0)
	, m_nPop_l(0)
	, m_nPop_r(0)
	, m_dVal_l(0)
	, m_dVal_r(0)
	, m_cVal_l(0)
	, m_cVal_r(0)
	, m_bEnable(TRUE)
	, m_dwTime(0)
	, m_old_max(0)
	, m_new_max(0)
	, m_bPlaySound(FALSE)
	, m_bChannelExited(FALSE)
	, m_strlog(_T(""))
	, m_bIsFullSerccn(FALSE)
	, m_bOldPlaySound(FALSE)
{
	InitializeCriticalSection(&m_Critical);
}


CChannel::~CChannel(void)
{
	Destory();
	DeleteCriticalSection(&m_Critical);
}

//////////////////////////////音柱绘制部分////////////////////////////////////////////

// 开始绘制音频柱图
void CChannel::DrawAudioVal( HWND hWnd )
{
	m_hWnd = hWnd;

	m_nColorBarPixcel = 4;		// 彩条像素高度
	m_nSeparatorPixcel = 1;		// 间隙像素高度
	m_nSeparatorWidth = 4;		// 间隙像素宽度
	m_nColorBarWidth = ( m_rtAudio.right-m_rtAudio.left - m_nSeparatorWidth - 4 )/2;			// 彩条像素宽度

	m_cColorBar = (m_rtAudio.bottom-m_rtAudio.top) /( m_nColorBarPixcel + m_nSeparatorPixcel );	// 彩条总数

	m_nVal_l = 0;	m_nVal_r = 0;
	m_nPop_l = 0;	m_nPop_r = 0;

	DWORD dwID;
	m_hThreadPop = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)jmp_ThreadPop, this, 0, &dwID );
	CString szLog;
	szLog.Format(_T("CreateThread%X"), m_hThreadPop);
	WriteRunLog(szLog);
}

// 绘制音频柱图
void CChannel::DrawVal(HDC hDC, RECT rect)
{
	//	::EnterCriticalSection(&m_Critical);

	//创建内存绘图设备
	HDC memDC = ::CreateCompatibleDC( hDC );
	HBITMAP memBitmap = ::CreateCompatibleBitmap( hDC, rect.right-rect.left, rect.bottom-rect.top );
	memBitmap = (HBITMAP)::SelectObject( memDC, memBitmap );

	inerDrawValue( memDC, rect.right-rect.left, rect.bottom-rect.top );

	//把内存绘图拷贝到屏幕
	::BitBlt( hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom- rect.top, memDC, 0, 0, SRCCOPY );

	memBitmap = (HBITMAP)::SelectObject( memDC, memBitmap );
	if (!::DeleteObject( memBitmap ))
	{
		CString szLog;
		szLog.Format(_T("DeleteObject"));
		WriteRunLog(szLog);
	}
	if (!::DeleteDC( memDC ))
	{
		CString szLog;
		szLog.Format(_T("DeleteDC"));
		WriteRunLog(szLog);
	}
}
void CChannel::inerDrawValue(HDC hDC, int cx, int cy)
{
	// 构造背景颜色刷
	HBRUSH hBackBrush = ::CreateSolidBrush( RGB(0,0,0) );
	// 绘制背景
	RECT rc = { 0, 0, cx, cy };
	::FillRect( hDC, &rc, hBackBrush );
	if (!::DeleteObject( hBackBrush ))
	{
		CString szLog;
		szLog.Format(_T("DeleteObject"));
		WriteRunLog(szLog);
	}

	DrawOneTrack( hDC, m_nVal_l, m_nPop_l,		// 左声道
		(cx-m_nSeparatorWidth)/2-m_nColorBarWidth,
		0,
		(cx-m_nSeparatorWidth)/2,
		cy );
	DrawOneTrack( hDC, m_nVal_r, m_nPop_r,		// 右声道
		(cx+m_nSeparatorWidth)/2, 0,
		(cx+m_nSeparatorWidth)/2+m_nColorBarWidth,
		cy );
}
// 绘制一个声道的数据
void CChannel::DrawOneTrack( HDC hDC, int nVal, int &nPop, int left, int top, int right, int bottom )
{
	int gLine = m_cColorBar * 60 / 100;				// 绿色彩条行数
	int yLine = m_cColorBar * 30 / 100;				// 黄色彩条行数
	int rLine = m_cColorBar - gLine - yLine;		// 红色彩条行数

	int pos = bottom;
	for( int y=0; y<m_cColorBar; y++ )
	{
		RECT rc;
		rc.left = left;		rc.top = pos-m_nColorBarPixcel;
		rc.right = right;	rc.bottom = pos;

		pos -= m_nColorBarPixcel + m_nSeparatorPixcel;		// 下次绘制的位置

		COLORREF col;
		if( y == nPop && nVal != nPop && 0 != nPop )
		{
			col = RGB( 255, 255, 255 );			// 白色 弹跳值
		}
		else if( 0<=y && y<gLine )				// 绿色
		{
			if( y<nVal )	col = RGB( 0, 255, 0 );
			else			col = RGB( 0, 48, 0 );	//RGB( 0, 96, 0 );
		}
		else if( gLine<=y && y<gLine+yLine )	// 黄色
		{
			if( y<nVal )	col = RGB( 255, 255, 0 );
			else			col = RGB( 48, 48, 0 );	//RGB( 96, 96, 0 );
		}
		else									// 红色
		{
			if( y<nVal )	col = RGB( 255, 0, 0 );
			else			col = RGB( 48, 0, 0 );	//RGB( 96, 0, 0 );		
		}

		HBRUSH hBrush = (HBRUSH)::CreateSolidBrush( col );
		::FillRect( hDC, &rc, hBrush );
		if (!::DeleteObject( hBrush ))
		{
			CString szLog;
			szLog.Format(_T("DeleteObject"));
			WriteErrorLog(szLog);
		}
	}
}

// 设置音频值(0~255)
void CChannel::put_AudioVal( int newVal_l, int newVal_r,DWORD realVal )
{
	newVal_l = newVal_l * m_cColorBar / 256;
	newVal_r = newVal_r * m_cColorBar / 256;

	//	::EnterCriticalSection(&m_Critical);

	if( newVal_l > m_nVal_l )
	{
		m_nVal_l = newVal_l;
		if( m_nVal_l >= m_nPop_l )
		{
			m_dVal_l = 0;			// 刚更新，设置则弹跳下落加速度为0
			m_cVal_l = 0;			// 加速度计数器
			m_nPop_l = m_nVal_l+1;
		}
	}
	if( newVal_r > m_nVal_r )
	{
		m_nVal_r = newVal_r;
		if( m_nVal_r >= m_nPop_r )
		{
			m_dVal_r = 0;			// 刚更新，设置则弹跳下落加速度为0
			m_cVal_r = 0;			// 加速度计数器
			m_nPop_r = m_nVal_r+1;
		}
	}
}

void CChannel::AudioCallback2( DWORD val_l, DWORD val_r )
{
	if( 0 == m_dwTime )	m_dwTime = ::GetTickCount();

	////////  每间隔10秒钟，统计一次最高音频幅度，作为调整参考 /////////////
	if( val_l > m_new_max )	m_new_max = val_l;
	if( val_r > m_new_max )	m_new_max = val_r;

	if( ::GetTickCount() - m_dwTime > 10*1000 )	// 10秒刷新一次最大值，调整100%的幅度
	{
		m_old_max = m_new_max;	if( m_old_max < 3000 )	m_old_max = 3000;
		m_dwTime = ::GetTickCount();
		m_new_max = 0;
	}

	int cval_l = (int)( val_l * 256 / (float)m_old_max );		// 调制音频到 0-255 的范围中
	if( cval_l>255 )	cval_l = 255;

	int cval_r = (int)( val_r * 256 / (float)m_old_max );		// 调制音频到 0-255 的范围中
	if( cval_r>255 )	cval_r = 255;

	put_AudioVal( cval_l, cval_r, m_new_max );
}


DWORD WINAPI CChannel::jmp_ThreadPop( LPVOID pParam )
{
	((CChannel *)pParam)->ThreadPop();
	return 0;
}

void CChannel::ThreadPop(void)
{
	static const int dTime[]={ 10, 8, 6, 5, 4, 3, 2, 1  };
	m_dVal_l = 0;	m_dVal_r = 0;
	int nCount = 0;

	while( !m_bChannelExited )
	{
		::Sleep( 12 );

		if (m_bIsFullSerccn)
		{//如果是全屏，不绘制音柱
			::Sleep(50);
			continue;
		}
			

		bool bDirty = false;
		nCount ++;

		
		::EnterCriticalSection(&m_Critical);
		if (!m_bEnable)
		{
			::LeaveCriticalSection(&m_Critical);
			continue;
		}

		if( nCount >= 3 )
		{
			if( m_nVal_l )	m_nVal_l--;
			if( m_nVal_r )	m_nVal_r--;
			nCount = 0;		bDirty = true;
		}

		m_cVal_l ++;		m_cVal_r ++;		// 下落计数器累计
		if( m_cVal_l >= dTime[m_dVal_l] )
		{
			if( m_dVal_l +1 < sizeof(dTime)/sizeof(int)-1 )	m_dVal_l++;
			m_cVal_l = 0;	bDirty = true;
			m_nPop_l--;	if( m_nPop_l<=m_nVal_l )	m_nPop_l = 0;
		}
		if( m_cVal_r >= dTime[m_dVal_r] )
		{
			if( m_dVal_r +1 < sizeof(dTime)/sizeof(int)-1 )	m_dVal_r++;
			m_cVal_r = 0;	bDirty = true;
			m_nPop_r--;	if( m_nPop_r<=m_nVal_r )	m_nPop_r = 0;
		}
		::LeaveCriticalSection(&m_Critical);
		

		if( bDirty )
		{
			HDC hDC = ::GetDC( m_hWnd );

			DrawVal( hDC, m_rtAudio );			// 重新绘制
			if (0 == ::ReleaseDC( m_hWnd, hDC ))
			{
				CString szLog;
				szLog.Format(_T("ReleaseDC"));
				WriteRunLog(szLog);
			}
		}
	}
}

BOOL CChannel::InitAV(CString url, int port)
{
	if( !m_hWnd )
		return FALSE;

	m_pD3DRender = new CD3DRender;
	ASSERT(m_pD3DRender);
	
	int nNormalWidth = 1280;
	int nNormalHeight = 720;
	nNormalWidth = m_rtVideo.Width() > nNormalWidth ? m_rtVideo.Width() : nNormalWidth;
	nNormalHeight = m_rtVideo.Height() > nNormalHeight ? m_rtVideo.Height() : nNormalHeight;

	BOOL bRet = m_pD3DRender->init_render(m_hWnd,nNormalWidth,nNormalHeight,1);
	if ( bRet )
	{
		DrawAudioVal(m_hWnd); //初始化音柱绘制
//		m_pQueue = new CCircularQueue; //
		m_pDecoder = new CDecoder(url,port,(void*)this); //初始化解码器
		ASSERT(m_pDecoder);
		m_pDecoder->InitDecoder(nNormalWidth,nNormalHeight);//
	}else{
		m_strlog.Format(_T("%s:%d InitDDraw init error!"), url, port);
		WriteErrorLog(m_strlog);
	}

	m_strlog.Format(_T("InitAV\n"));
	WriteRunLog(m_strlog);
	return bRet;
}

// void CChannel::SaveVideoData( const FFVideoData* pVFD, const FFVideoInfo *pVFI )
// {
// 	_tagVideoHead VideoHead;
// 	memset(&VideoHead, 0, sizeof(VideoHead));
// 
// 	VideoHead.height = pVFI->height;
// 	VideoHead.width = pVFI->width;
// 	VideoHead.linesizeY = pVFD->linesizey;
// 	VideoHead.linesizeU = pVFD->linesizeu;
// 	VideoHead.linesizeV = pVFD->linesizev;
// 
// 	m_pQueue->append(&VideoHead, sizeof(VideoHead), (void*)pVFD->pY, pVFD->linesizey*pVFI->height,
// 				(void*)pVFD->pU,pVFD->linesizeu*pVFI->height/2,
// 				(void*)pVFD->pV, pVFD->linesizev*pVFI->height/2);
// }

// BOOL CChannel::DrawVideo( void )
// {
// 	FFVideoData vfd;
// 	FFVideoInfo vfi;
// 	memset(&vfd, 0, sizeof(vfd));
// 	memset(&vfi, 0, sizeof(vfi));
// 
// 	CRect rect;
// 	if (m_bIsFullSerccn)
// 	{
// 		rect = m_rectFullSreen;
// 	}
// 	else{
// 		 rect = m_rtVideo;
// #if !USE_D3D_RENDER
// 		CWnd::FromHandle(m_hWnd)->ClientToScreen(rect);
// #endif
// 	}
// 	
// 
// 	int size=0;
// 	_tagVideoHead *pVideoHead=NULL;
// 	pVideoHead = (_tagVideoHead*)m_pQueue->get(&size);
// 
// 	if (!pVideoHead || size == 0)
// 	{
// 		//没有数据，绘制蓝色背景
// //		m_pDDraw->InitVideo(rect, 0, 0, 255);  //16-01-05 注释掉，视频帧率不足，会出现蓝色和视频相间的情况
// 		return FALSE;
// 	}
// 		
// 	vfi.width = pVideoHead->width;
// 	vfi.height = pVideoHead->height;
// 	vfd.linesizey = pVideoHead->linesizeY;
// 	vfd.linesizeu = pVideoHead->linesizeU;
// 	vfd.linesizev = pVideoHead->linesizeV;
// 	vfd.pY = (uint8_t*)((char*)pVideoHead+sizeof(_tagVideoHead));
// 	vfd.pU = (uint8_t *)((char*)pVideoHead + sizeof(_tagVideoHead) + vfd.linesizey*vfi.height);
// 	vfd.pV = (uint8_t *)((char*)pVideoHead + sizeof(_tagVideoHead) + vfd.linesizey*vfi.height
// 		+vfd.linesizeu*vfi.height/2);
// 	BOOL bRet = FALSE;
// #if USE_D3D_RENDER
// 	bRet = m_pD3DRender->render_one_frame(&vfd,&vfi,1,rect);
// #else
// 	if ( m_bIsFullSerccn )
// 		 bRet = m_pDDraw->RenderYUV(rect, &vfd, &vfi, FALSE);
// 	else
// 		bRet = m_pDDraw->RenderYUV(rect, &vfd, &vfi, m_bPlaySound);
// //	bRet = m_pD3Draw->render_one_frame(&vfd, &vfi, 0, rect);
// #endif
// 	m_pQueue->back(pVideoHead,size);
// 
// 	return bRet;
// }

BOOL CChannel::DrawVideo(const FFVideoData* pVFD, const FFVideoInfo *pVFI)
{
	if (m_bChannelExited) //退出标记
		return FALSE;

	CRect rect;
	if (m_bIsFullSerccn)
	{
		rect = m_rectFullSreen;
	}
	else{
		rect = m_rtVideo;
	}

	bool bRet = false;
	if ( m_bIsFullSerccn )
		bRet = m_pD3DRender->render_one_frame((FFVideoData*)pVFD, (FFVideoInfo*)pVFI, 1, rect, false);
	else
		bRet = m_pD3DRender->render_one_frame((FFVideoData*)pVFD, (FFVideoInfo*)pVFI, 1, rect, m_bPlaySound == FALSE ? false : true);
	

	return bRet == false ? FALSE : TRUE;

}

BOOL CChannel::IsInChannelRect( CPoint &point ) const
{
	if ( m_rtAudio.PtInRect(point) || m_rtVideo.PtInRect(point) )
		return TRUE;
	else
		return FALSE;
}

void CChannel::putVideoOrAudioRect( int x, int y, int w, int h, BOOL bVideo/*=TRUE*/ )
{
	if ( bVideo)
	{
		m_rtVideo.left = x;
		m_rtVideo.top = y;
		m_rtVideo.right = x+w;
		m_rtVideo.bottom = y+h;

		m_rectFullSreen.left = 0;
		m_rectFullSreen.top = 0;
		m_rectFullSreen.right = ::GetSystemMetrics(SM_CXSCREEN);
		m_rectFullSreen.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		m_rtAudio.left = x;
		m_rtAudio.top = y;
		m_rtAudio.right = x+w;
		m_rtAudio.bottom = y+h;
	}
}

CRect CChannel::GetVideoOrAudioRect( BOOL bVideo/*=TRUE*/ ) const
{
	if ( bVideo )
		 return m_rtVideo;
	else
		return m_rtAudio;
}

void CChannel::Destory()
{
	m_bChannelExited = TRUE;
	m_strlog.Format(_T("Destory 调用！"));
	WriteRunLog(m_strlog);

	DWORD dwRet = WaitForSingleObject(m_hThreadPop, 2*1000);
	if ( dwRet == WAIT_TIMEOUT )
	{
		m_strlog.Format(_T("WaritForSingleObject timeout"));
		WriteErrorLog(m_strlog);

		if( m_hThreadPop )
		{
			::TerminateThread( m_hThreadPop, -1 );
			::CloseHandle( m_hThreadPop );
			m_hThreadPop = INVALID_HANDLE_VALUE;
		}
	}

	if (m_pD3DRender)
	{
		m_pD3DRender->destory_render();
		delete m_pD3DRender;
		m_pD3DRender = NULL;
	}

	if ( m_pDecoder )
	{
		m_pDecoder->UnInitDecoder();
		delete m_pDecoder;
		m_pDecoder=NULL;
	}
// 	if ( m_pQueue )
// 	{
// 		delete m_pQueue;
// 		m_pQueue=NULL;
// 	}
}

void CChannel::SetDecoderLastTime(DWORD dwTime, int type)
{
	if (0 == type)
		m_pDecoder->SetLastAudioTime(dwTime);
	else if (1 == type)
		m_pDecoder->SetLastVideoTime(dwTime);
	else if (2 == type)
		m_pDecoder->SetLastTsTime(dwTime);
	else
		TRACE("type set value error!\n");
}

void CChannel::SetDecoderNeedRestart(BOOL bNeed, int nCondition)
{
	m_pDecoder->SetRestartCondition(bNeed, nCondition);
}

void CChannel::DetecteDecoderRestart(void)
{//检测解码器是否需要重启
	if (m_pDecoder->IsNeedRestart())
		m_pDecoder->RestartDecoder();
}

void CChannel::SetFullScreen(BOOL bFull)
{
	m_bIsFullSerccn = bFull; 
	m_bOldPlaySound = m_bPlaySound; 
}
