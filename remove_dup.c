#include<stdio.h>
#define SIZE 10

void remove_dup(int *arr)
{
	int i=0,j=0,l=0,flag=0,k=0;
	int new_arr[SIZE]={0};
	for(i=0;i<SIZE;i++)
	{
		flag=1;
		for(j=0;j<l;j++)
		{
			if(new_arr[j]==arr[i])
			{
				flag=0;
				break;
			}
		}
		if(flag)
		{
			new_arr[l++]=arr[i];
		}
	}
	printf("after remove duplicates: ");
	for(i=0;i<SIZE;i++)
	printf("%d ",new_arr[i]);
}

int main()
{
	int arr[SIZE]={0};

	printf("enter numbers between 10 ... \n");
	for(int i=0;i<SIZE;i++)
	{
		scanf("%d",&arr[i]);
	}

	remove_dup(arr);
	return 0;
}
