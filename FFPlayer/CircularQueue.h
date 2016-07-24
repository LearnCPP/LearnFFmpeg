/////////////////////////////////////////////////////////////////////
// ����: 
// ����: ctx
// ����: ѭ������
// ��Ҫ������append,get,back
// ����: 2015-01-16
// �汾: 1.0
// �޸�:
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
	CRITICAL_SECTION	m_Critical;	// ͬ������

	int m_count;					// ����Ԫ�أ�������������
	int m_idHead;					// ͷ����
	int m_idTail;					// β����
	char ** m_queue;				// ����
	int * m_size;					// �������ߴ�

public:
	void append( void *p1, int size1, void *p2, int size2, void *p3, int size3, void *p4, int size4 );	// ׷������
	void * get( int * pSize );														// ��ȡ����,���뻺����
	void back( void *p, int size );													// �˻ػ�����ָ��
};
