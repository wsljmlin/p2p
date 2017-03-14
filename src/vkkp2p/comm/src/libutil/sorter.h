#pragma once

namespace adc
{
	template<typename T>
	void sort_bubble(T* arr,int n,bool ascending=true)
	{
		int i=0,j,k=n-1;
		T tmp;
		for(j=k;j>0;j=k)
		{
			k = 0;
			for(i=0;i<j;++i)
			{
				if((ascending&&arr[i]>arr[i+1]) ||((!ascending)&&arr[i]<arr[i+1]))
				{
					tmp = arr[i];
					arr[i] = arr[i+1];
					arr[i+1] = tmp;
					k = i;
				}
			}
		}
	}
};
