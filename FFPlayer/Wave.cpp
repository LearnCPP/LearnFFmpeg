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
	m_waveout.nChannels			=	2;		// ������
	m_waveout.nSamplesPerSec	=	16000;	// ������
	m_waveout.wBitsPerSample	=	16;		// ��������
	m_waveout.nAvgBytesPerSec	=	m_waveout.nChannels * m_waveout.nSamplesPerSec * (m_waveout.wBitsPerSample / 8);	// ÿ�벥���ֽ���
	m_waveout.nBlockAlign		=	m_waveout.wBitsPerSample/8 * m_waveout.nChannels;									// ���������ֽ���

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
		if( -1 == bRet )	bRight = false;		// �����˳�
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
		if( STILL_ACTIVE == dw )	// �̻߳�������
		{
			::WaitForSingleObject( m_hThreadWave, 1000 );
			::TerminateThread( m_hThreadWave, 0 );
		}
		::CloseHandle( m_hThreadWave );
		m_hThreadWave = NULL;
	}
	return true;
}
// ��������
void CWave::Play( char * pBuf, int nLen )
{
l_begin:
	::EnterCriticalSection( &m_Critical );
	int i;
	for(  i=0; i<BLOCK_COUNT; i++ )			// �������û���
	{
		if( -1 == m_flag[i] )	break;
	}
	if( i >= BLOCK_COUNT )						// û�����û��壬��������ɻ���
	{
		for( i=0; i<BLOCK_COUNT; i++ )
			if( 0 == m_flag[i] )	break;
		
		if( i >= BLOCK_COUNT )					// ��û�����û��壬Ҳû�����ɻ���
		{
			::LeaveCriticalSection( &m_Critical );
			return;
		}
	}

	if( m_hdr[i].dwBufferLength + nLen <= BLOCK_SIZE )	// ��������װ��
	{
		::memcpy( &m_hdr[i].lpData[m_hdr[i].dwBufferLength], pBuf, nLen );
		m_hdr[i].dwBufferLength += nLen;
		m_flag[i] = -1;									// ���ڻ�����

		::LeaveCriticalSection( &m_Critical );
		return;
	}

	if( -1 == m_flag[i] )			// ���ڻ���Ļ��������Ѿ�װ������������
	{
		m_flag[i] = 1;				// ���ڲ��ű��

		::waveOutPrepareHeader( m_hWaveOut, &m_hdr[i], sizeof(WAVEHDR) );
		::waveOutWrite( m_hWaveOut, &m_hdr[i], sizeof(WAVEHDR) );
		::LeaveCriticalSection( &m_Critical );
		goto l_begin;
	}
	else							// ���ɻ�����
	{
		::memcpy( &m_hdr[i].lpData[0], pBuf, nLen );
		m_hdr[i].dwBufferLength = nLen;
		m_flag[i] = -1;									// ���ڻ�����
	}
	::LeaveCriticalSection( &m_Critical );
	return;
}

// ��������������ɵ�ʱ��
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
