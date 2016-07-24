#include "StdAfx.h"
#include <stdio.h>

#include "circularqueue.h"

CCircularQueue::CCircularQueue(int nMaxCount)
{
	m_count = nMaxCount;

	m_queue = new char * [nMaxCount];		// 指针队列
	::memset( m_queue, 0, nMaxCount * sizeof(char *) );

	m_size = new int [nMaxCount];			// 缓冲长度
	::memset( m_size, 0, nMaxCount * sizeof(int) );

	m_idHead = 0;							// 头索引
	m_idTail = 0;							// 尾索引

	::InitializeCriticalSection( &m_Critical );
}

CCircularQueue::~CCircularQueue(void)
{
	{
		CLock lock( &m_Critical );

		for( int i=0; i<m_count; i++ )
		{
			if( m_queue[i] )	::GlobalFree( m_queue[i] );
		}
		delete [] m_queue;
		delete []m_size;

		::LeaveCriticalSection( &m_Critical );
	}
	::DeleteCriticalSection( &m_Critical );
}

// 追加数据
void CCircularQueue::append( void *p1, int size1, void *p2, int size2, void *p3, int size3, void *p4, int size4 )
{
	CLock lock( &m_Critical );

	int size = size1 + size2 + size3 + size4;
	if( 0 == size )	return;

	if( (m_idTail + 1)%m_count == m_idHead )	// 缓冲区满
	{
		m_idHead = (m_idHead + 1)%m_count;		// 则丢弃头数据，头索引+1
		m_idHead = (m_idHead + 1)%m_count;
	}

	/////////// 向 idTail 索引中，添加数据 ////////////////////
	if( m_size[m_idTail] < size )				// 缓冲区太小了
	{
		if( m_queue[m_idTail] )
		{
			::GlobalFree( m_queue[m_idTail] );		// 重新申请缓冲区
			m_queue[m_idTail] = NULL;
			m_size[m_idTail] = 0;
		}

		m_queue[m_idTail] = (char *)::GlobalAlloc( GPTR, size );
		if( NULL == m_queue[m_idTail] )	return;							// 申请内存失败
	}

	m_size[m_idTail] = size;					// 缓冲区尺寸
	char *p = m_queue[m_idTail];

	::memcpy( p, p1, size1 );	p += size1;
	::memcpy( p, p2, size2 );	p += size2;
	::memcpy( p, p3, size3 );	p += size3;
	::memcpy( p, p4, size4 );	//p += size4;

	m_idTail = (m_idTail + 1)%m_count;			// 尾索引+1
}

// 获取数据,剥离缓冲区
void * CCircularQueue::get( int * pSize )
{
	CLock lock( &m_Critical );

	if( m_idHead == m_idTail )	// 空队列
	{
		*pSize = 0;	return NULL;
	}

	///////////////// 剥离缓冲区 ////////////////////////
	*pSize = m_size[ m_idHead ];			
	m_size[ m_idHead ] = 0;					// 长度清零
	void *p = m_queue[ m_idHead ];
	m_queue[ m_idHead ] = NULL;				// 指针清空

	m_idHead = (m_idHead + 1)%m_count;		// 头索引+1
	return p;
}

// 退回缓冲区指针
void CCircularQueue::back( void *p, int size )
{
	CLock lock( &m_Critical );

	for( int i=m_idTail; i!=m_idHead; i=(i+1)%m_count )
	{
		if( 0 == m_size[i] && NULL == m_queue[i] )	// 队列中自由缓冲
		{
			m_size[i] = size;
			m_queue[i] = (char *)p;

			return;
		}
	}
	::GlobalFree( p );		// 无自由缓冲，则释放内存
}
