#include "StdAfx.h"
#include ".\wave.h"

#pragma comment( lib, "Winmm.lib" )

CWave::CWave(void)
{
	::InitializeCriticalSection( &m_Critical );

	m_hWaveOut = NULL;
	m_dwOutVal = 0;
	m_idThreadWave = 0;
	m_hThreadWave = NULL;

	m_waveout.cbSize			=	sizeof(WAVEFORMATEX);
	m_waveout.wFormatTag		=	WAVE_FORMAT_PCM;
	m_waveout.nChannels			=	2;		// 声道数
	m_waveout.nSamplesPerSec	=	16000;	// 采样率
	m_waveout.wBitsPerSample	=	16;		// 采样精度
	m_waveout.nAvgBytesPerSec	=	m_waveout.nChannels * m_waveout.nSamplesPerSec * (m_waveout.wBitsPerSample / 8);	// 每秒播放字节数
	m_waveout.nBlockAlign		=	m_waveout.wBitsPerSample/8 * m_waveout.nChannels;									// 样本对齐字节数

	for(int i=0;i<BLOCK_COUNT; i++ )
	{
		::memset( &m_hdr[i], 0, sizeof( WAVEHDR ) );
		m_hdr[i].lpData = new char [BLOCK_SIZE];
		m_flag[i] = 0;
	}
}

CWave::~CWave(void)
{
	Close();
	::EnterCriticalSection( &m_Critical );

	for( int i=0; i<BLOCK_COUNT; i++ )
	{
		try
		{
			delete [] m_hdr[i].lpData;
		}
		catch(...){}
	}

	::LeaveCriticalSection( &m_Critical );
	::DeleteCriticalSection( &m_Critical );
}

static DWORD WINAPI ThreadWave( LPVOID pParam )
{
	CWave * pWave = (CWave *)pParam;

	MSG msg;
	BOOL bRet;
	bool bRight = true;

	while( bRight && (bRet = GetMessage(&msg,0,0,0) ) )
	{
		if( -1 == bRet )	bRight = false;		// 错误退出
		else
			bRight = pWave->WaveMsg( msg.message, (WAVEHDR *)msg.lParam );
	}
	return 0;
}
bool CWave::WaveMsg( UINT message, WAVEHDR * pWH )
{
	bool bRight = true;
	::EnterCriticalSection( &m_Critical );

	switch( message )
	{
	case WOM_OPEN:		break;
	case WOM_CLOSE:		break;
	case WOM_DONE:		
		WomDone( pWH );
		break;
	}

	::LeaveCriticalSection( &m_Critical );
	return bRight;
}
bool CWave::Start(WAVEFORMATEX *pwaveformat)
{
	if( m_hWaveOut )	Close();

	m_hThreadWave = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ThreadWave, this, 0, &m_idThreadWave );

	::memcpy( &m_waveout, pwaveformat, sizeof(WAVEFORMATEX) );

	if( NULL == m_hWaveOut )
	{
		::waveOutOpen( &m_hWaveOut, WAVE_MAPPER, &m_waveout, m_idThreadWave, 0, CALLBACK_THREAD );
		::waveOutGetVolume( m_hWaveOut, &m_dwOutVal );
	}

	for( int i=0; i<BLOCK_COUNT; i++ )
	{
		m_flag[i] = 0;
	}
	return true;
}

bool CWave::Close(void)
{
	::EnterCriticalSection( &m_Critical );
	if( m_hWaveOut )
	{
		::waveOutReset( m_hWaveOut );
		::waveOutClose( m_hWaveOut );
		m_hWaveOut = NULL;
	}
	::LeaveCriticalSection( &m_Critical );

	if( m_hThreadWave )
	{
		::PostThreadMessage( m_idThreadWave, WM_QUIT, 0, 0 );
		DWORD dw;
		::GetExitCodeThread( m_hThreadWave, &dw );
		if( STILL_ACTIVE == dw )	// 线程还在运行
		{
			::WaitForSingleObject( m_hThreadWave, 1000 );
			::TerminateThread( m_hThreadWave, 0 );
		}
		::CloseHandle( m_hThreadWave );
		m_hThreadWave = NULL;
	}
	return true;
}
// 播放声音
void CWave::Play( char * pBuf, int nLen )
{
l_begin:
	::EnterCriticalSection( &m_Critical );
	int i;
	for(  i=0; i<BLOCK_COUNT; i++ )			// 查找在用缓冲
	{
		if( -1 == m_flag[i] )	break;
	}
	if( i >= BLOCK_COUNT )						// 没有在用缓冲，则查找自由缓冲
	{
		for( i=0; i<BLOCK_COUNT; i++ )
			if( 0 == m_flag[i] )	break;
		
		if( i >= BLOCK_COUNT )					// 既没有在用缓冲，也没有自由缓冲
		{
			::LeaveCriticalSection( &m_Critical );
			return;
		}
	}

	if( m_hdr[i].dwBufferLength + nLen <= BLOCK_SIZE )	// 缓冲区能装下
	{
		::memcpy( &m_hdr[i].lpData[m_hdr[i].dwBufferLength], pBuf, nLen );
		m_hdr[i].dwBufferLength += nLen;
		m_flag[i] = -1;									// 正在缓冲标记

		::LeaveCriticalSection( &m_Critical );
		return;
	}

	if( -1 == m_flag[i] )			// 正在缓冲的缓冲区，已经装不下新数据了
	{
		m_flag[i] = 1;				// 正在播放标记

		::waveOutPrepareHeader( m_hWaveOut, &m_hdr[i], sizeof(WAVEHDR) );
		::waveOutWrite( m_hWaveOut, &m_hdr[i], sizeof(WAVEHDR) );
		::LeaveCriticalSection( &m_Critical );
		goto l_begin;
	}
	else							// 自由缓冲区
	{
		::memcpy( &m_hdr[i].lpData[0], pBuf, nLen );
		m_hdr[i].dwBufferLength = nLen;
		m_flag[i] = -1;									// 正在缓冲标记
	}
	::LeaveCriticalSection( &m_Critical );
	return;
}

// 当缓冲区播放完成的时候
void CWave::WomDone(PWAVEHDR pWaveHdr)
{
	::EnterCriticalSection( &m_Critical );

	if( m_hWaveOut )
		::waveOutUnprepareHeader( m_hWaveOut, pWaveHdr, sizeof(WAVEHDR) );

	for( int i=0; i<BLOCK_COUNT; i++ )
	{
		if( pWaveHdr == &m_hdr[i] )
		{
			m_flag[i] = 0;		m_hdr[i].dwBufferLength = 0;
			break;
		}
	}
	::LeaveCriticalSection( &m_Critical );
}
