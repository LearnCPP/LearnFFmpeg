/////////////////////////////////////////////////////////////////////
// 工程: 
// 作者: ctx
// 描述: 循环队列
// 主要函数：append,get,back
// 日期: 2015-01-16
// 版本: 1.0
// 修改:
/////////////////////////////////////////////////////////////////////
#pragma once

class CLock
{
private:
	CRITICAL_SECTION *m_pCritical;
public:
	CLock( CRITICAL_SECTION *pCritical )
	{
		m_pCritical = pCritical;
		::EnterCriticalSection( m_pCritical );
	}
	~CLock()
	{
		::LeaveCriticalSection( m_pCritical );
	}
};

class CCircularQueue
{
public:
	CCircularQueue(int nMaxCount=20);
	~CCircularQueue(void);
private:
	CRITICAL_SECTION	m_Critical;	// 同步对象

	int m_count;					// 队列元素（缓冲区）个数
	int m_idHead;					// 头索引
	int m_idTail;					// 尾索引
	char ** m_queue;				// 队列
	int * m_size;					// 缓冲区尺寸

public:
	void append( void *p1, int size1, void *p2, int size2, void *p3, int size3, void *p4, int size4 );	// 追加数据
	void * get( int * pSize );														// 获取数据,剥离缓冲区
	void back( void *p, int size );													// 退回缓冲区指针
};
