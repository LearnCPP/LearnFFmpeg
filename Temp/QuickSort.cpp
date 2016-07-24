#include"stdafx.h"
#include "sort.h"

int AdjustArray(int s[], int l, int r)
{
	int i = l, j = r;
	int base = s[i]; //基准数为左边第一个
	while ( i < j )
	{
		//从右向左寻找小于base的数来填s[i]
		while (i<j && s[j]>base)
			j--;
		if (i<j)
		{
			s[i] = s[j];
			i++;
		}
		
		//从左往右寻找大于base的数来填s[j]
		while (i < j && s[i] < base)
			i++;
		if (i<j)
		{
			s[j] = s[i];
			j--;
		}
	}
	//退出时，i=j把base填到这个坑中
	s[i] = base;

	return i; //返回调整后基准数位置
}

//分治法算法
void quicksort1(int s[], int l, int r)
{
	if (l<r)
	{
		int i = AdjustArray(s, l, r);
		quicksort1(s, l, i-1);
		quicksort1(s, i+1, r);
	}
}

//快速排序
void quicksort(int s[], int l, int r)
{
	if (l<r)
	{
		int i = l, j = r;
		int base = s[i];

		while ( i < j )
		{
			//从右向左找，小于base的数，填到s[i]中
			while (i < j && s[j] >= base)
				j--;
			if (i < j)
			{
				s[i++] = s[j];
			}
			//从左往右找，大于base的数，填到s[j]中
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