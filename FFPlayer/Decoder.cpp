#include "StdAfx.h"
#include "Decoder.h"
#include "Channel.h"
#include "Wave.h"

#pragma comment(lib,"FFDecoder.lib")

CWave g_wave;//音频播放对象
WAVEFORMATEX g_waveFormatex; //波形音频流音频结构体

CDecoder::CDecoder(CString url, int port, void* pthis)
	: m_strUrl(url)
	, m_nPort(port)
	, m_psd(NULL)
	, m_pParent(pthis)
	, m_strlog(_T(""))
	, m_bAudio(FALSE)
	, m_bVideo(FALSE)
	, m_bTs(FALSE)
	, m_dwAudioLastTime(0)
	, m_dwVideoLastTime(0)
	, m_dwTSLastTime(0)
	, m_dwStartTime(GetTickCount())
	, m_bIsRestart(FALSE)
	, m_nWidth(0)
	, m_nHight(0)
{
	//sd_global_init(); //全局初始化
}


CDecoder::~CDecoder(void)
{
	//sd_global_unInit();
}

BOOL CDecoder::InitDecoder(int nWidth, int nHeight)
{

	if ( !m_psd )
	{
		//sd_alloc_context(&m_psd);
		FF_Alloc_Context(&m_psd);
		char szUrl[100] = { 0 };
		if ( m_nPort == -1 )
		{//非udp方式
			sprintf_s(szUrl, 100, "%s", m_strUrl);
			//sd_init(m_psd, szUrl, NULL, 0, 65535, 2 * 1024 * 1024, 0, 0, true, true);
			FF_Init_Context(m_psd, szUrl, m_pParent, VideoCallback, AudioCallback, TsCallback, true, true);
		}
		else{//UDP 方式
			sprintf_s(szUrl, 100, "udp://%s:%d", m_strUrl, m_nPort);
			//sd_init(m_psd, szUrl, NULL, m_nPort, 65535, 2 * 1024 * 1024, 0, 0, true, true);
			FF_Init_Context(m_psd, szUrl, m_pParent, VideoCallback, AudioCallback, TsCallback, true, true);
		}
		//FF_SetVideoRect(m_psd,1280, 720);
		//sd_set_trans_api(m_psd,VideoCallback, AudioCallback, TsCallback, m_pParent);
		//sd_start(m_psd);
		m_nWidth = nWidth;
		m_nHight = nHeight;
		FF_SetVideoRect(m_psd, nWidth, nHeight);
		FF_Start(m_psd);
	}

	return TRUE;
}

void CDecoder::UnInitDecoder( void )
{
	if ( m_psd )
	{
		//sd_stop(m_psd);
		//sd_free_context(m_psd);
		FF_Stop(m_psd);
		FF_Free_Context(m_psd);

		m_psd = NULL;
	}
	g_wave.Close();
	ZeroMemory(&g_waveFormatex, sizeof(WAVEFORMATEX));
}

int CDecoder::VideoCallback( void * userdata, const FFVideoData *vfd, const FFVideoInfo *vfi )
{
	CChannel* pChannel = (CChannel*) userdata;

// 	pChannel->SetDecoderNeedRestart(TRUE, 1);
// 	pChannel->SetDecoderLastTime(GetTickCount(), 1);

	//pChannel->SaveVideoData(vfd,vfi);
	pChannel->DrawVideo(vfd,vfi);

	return 0;
}

int CDecoder::AudioCallback( void * userdata, const FFAudioData *afd, const FFAudioInfo *afi )
{
	/*static BOOL bFirst = TRUE;
	if ( bFirst )
	{
	AfxMessageBox(_T("AudioCallback"));
	bFirst = FALSE;
	}*/

	CChannel* pChannel = (CChannel*) userdata;
/*
	pChannel->SetDecoderNeedRestart(TRUE, 0);
	pChannel->SetDecoderLastTime(GetTickCount(), 0);*/

	if ( pChannel->GetAudioPlay() )
	{
		if ( g_waveFormatex.wBitsPerSample != afi->bit_pre_sample ||
			g_waveFormatex.nChannels != afi->channels ||
			g_waveFormatex.nSamplesPerSec != afi->sample_rate)
		{
			g_wave.Close();

			g_waveFormatex.cbSize = sizeof(WAVEFORMATEX);
			g_waveFormatex.nChannels = afi->channels;
			g_waveFormatex.nSamplesPerSec = afi->sample_rate; //采样率
			g_waveFormatex.wFormatTag = WAVE_FORMAT_PCM;
			g_waveFormatex.wBitsPerSample = afi->bit_pre_sample;//采样精度
			g_waveFormatex.nBlockAlign = afi->channels*afi->bit_pre_sample / 8;
			g_waveFormatex.nAvgBytesPerSec = afi->sample_rate/
				(afi->channels*afi->bit_pre_sample / 8);

			g_wave.Start(&g_waveFormatex);
		}
		g_wave.Play((char*)afd->data, afd->length);
	}
	/////////////////////// 只发送柱图信息 //////////////
	DWORD val_l, val_r;

	const char * lpPCM = (const char *)afd->data;
	int i, nLen = afd->length;
	if( 2 == afi->channels )		// 双声道
	{
		short int l_min=0, l_max=0, r_min=0, r_max=0;

		switch (afi->bit_pre_sample)		// 采样精度
		{
		case 8:
			for( i=0; i<nLen/2; i++ )
			{
				char t = *((char*)&lpPCM[i*2+0]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;

				t = *((char*)&lpPCM[i*2+1]);
				r_min = t<r_min? t:r_min;
				r_max = t>r_max? t:r_max;
			}
			break;
		case 16:
			for( i=0; i<nLen/4; i++ )
			{
				short int t = *((short int *)&lpPCM[i*4+0]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;

				t = *((short int *)&lpPCM[i*4+2]);
				r_min = t<r_min? t:r_min;
				r_max = t>r_max? t:r_max;
			}
			break;
		case 24:
			for( i=0; i<nLen/6; i++ )
			{
				short int t = *((short int *)&lpPCM[i*6+1]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;

				t = *((short int *)&lpPCM[i*6+4]);
				r_min = t<r_min? t:r_min;
				r_max = t>r_max? t:r_max;
			}
			break;
		case 32:
			for( i=0; i<nLen/8; i++ )
			{
				short int t = *((short int *)&lpPCM[i*8+2]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;

				t = *((short int *)&lpPCM[i*8+6]);
				r_min = t<r_min? t:r_min;
				r_max = t>r_max? t:r_max;
			}
			break;
		default:	return 0;					// 其它采样精度不考虑
		}
		val_l = l_max - l_min;
		val_r = r_max - r_min;
	}
	else if( 1 == afi->channels )		// 单声道
	{
		short int l_min=0, l_max=0;

		switch (afi->bit_pre_sample)		// 采样精度
		{
		case 8:
			for( i=0; i<nLen; i++ )
			{
				char t = *((char*)&lpPCM[i+0]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;
			}
			break;
		case 16:
			for( i=0; i<nLen/2; i++ )
			{
				short int t = *((short int *)&lpPCM[i*2+0]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;
			}
			break;
		case 24:
			for( i=0; i<nLen/3; i++ )
			{
				short int t = *((short int *)&lpPCM[i*3+1]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;
			}
			break;
		case 32:
			for( i=0; i<nLen/4; i++ )
			{
				short int t = *((short int *)&lpPCM[i*4+2]);
				l_min = t<l_min? t:l_min;
				l_max = t>l_max? t:l_max;
			}
			break;
		default:	return 0;					// 其它采样精度不考虑
		}
		val_l = l_max - l_min;
		val_r = val_l;
	}
	else return 0;	// 非单声道，非双声道 不考虑

	//调用显示音频
	pChannel->AudioCallback2(val_l, val_r);

	return 0;
}

int CDecoder::TsCallback( void *userdata, const uint8_t*, int length )
{
	//暂不做处理

	return 0;
}
//重启解码器
BOOL CDecoder::RestartDecoder(void)
{
	BOOL bResult = FALSE;
	DWORD dwTime = GetTickCount() - m_dwStartTime;
	TRACE("dwTime =%d\n", dwTime);
	if (m_bIsRestart ||  !(dwTime > 60 * 1000))
		return bResult;

	m_bIsRestart = TRUE;
	UnInitDecoder();
	Sleep(50);
	m_dwAudioLastTime = m_dwVideoLastTime = m_dwTSLastTime = 0;
	m_dwStartTime = GetTickCount();

	bResult = InitDecoder(m_nWidth,m_nHight);

	m_bIsRestart = FALSE;

	return bResult;
}

BOOL CDecoder::IsNeedRestart(void)
{
	BOOL bResult = FALSE;
	//正在重启
	if (m_bIsRestart)
		return bResult;

	//解码器启动60秒后，再检测是否重启
	if (m_bAudio && m_dwAudioLastTime - m_dwStartTime > 60*1000 && (::GetTickCount() - m_dwAudioLastTime > 10*1000) )
	{//10秒没有收到音频回调重启
			bResult = TRUE;
			TRACE(_T("m_dwAudioLastTime =%d \n"), m_dwAudioLastTime);
	}
	if (m_bVideo && m_dwVideoLastTime -m_dwStartTime > 60 * 1000 && (::GetTickCount() - m_dwVideoLastTime > 15 * 1000))
	{//15秒没有收到视频回调重启
		bResult = TRUE;
		TRACE(_T("m_dwVideoLastTime =%d \n"), m_dwVideoLastTime);
	}
	if (m_bTs && m_dwTSLastTime - m_dwStartTime > 60 * 1000 && (::GetTickCount() - m_dwTSLastTime > 10 * 1000))
	{
		bResult = TRUE;
		TRACE(_T("m_dwTSLastTime =%d \n"), m_dwTSLastTime);
	}

	return bResult;
}

void CDecoder::SetRestartCondition(BOOL bNeed, int nCondition)
{
	if (0 == nCondition)
		m_bAudio = bNeed;
	else if (1 == nCondition)
		m_bVideo = bNeed;
	else if (2 == nCondition)
		m_bTs = bNeed;
	else
		TRACE("Set nCondeition is error\n");
}
