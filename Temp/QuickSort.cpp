#include"stdafx.h"
#include "sort.h"

int AdjustArray(int s[], int l, int r)
{
	int i = l, j = r;
	int base = s[i]; //��׼��Ϊ��ߵ�һ��
	while ( i < j )
	{
		//��������Ѱ��С��base��������s[i]
		while (i<j && s[j]>base)
			j--;
		if (i<j)
		{
			s[i] = s[j];
			i++;
		}
		
		//��������Ѱ�Ҵ���base��������s[j]
		while (i < j && s[i] < base)
			i++;
		if (i<j)
		{
			s[j] = s[i];
			j--;
		}
	}
	//�˳�ʱ��i=j��base��������
	s[i] = base;

	return i; //���ص������׼��λ��
}

//���η��㷨
void quicksort1(int s[], int l, int r)
{
	if (l<r)
	{
		int i = AdjustArray(s, l, r);
		quicksort1(s, l, i-1);
		quicksort1(s, i+1, r);
	}
}

//��������
void quicksort(int s[], int l, int r)
{
	if (l<r)
	{
		int i = l, j = r;
		int base = s[i];

		while ( i < j )
		{
			//���������ң�С��base�������s[i]��
			while (i < j && s[j] >= base)
				j--;
			if (i < j)
			{
				s[i++] = s[j];
			}
			//���������ң�����base�������s[j]��
			while (i < j && s[i] < base)
				i++;
			if (i < j)
			{
				s[j--] = s[i];
			}
		}
		
		s[i] = base;
		quicksort(s, l, i - 1);
		quicksort(s, i + 1, r);
	}
}