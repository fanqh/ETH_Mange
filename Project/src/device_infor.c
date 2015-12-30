
#include "device_config.h"
#include "string.h"
#include "main.h"

Device_t Dev_list[SUPPORT_DEV_ONLINE_MAX];
// -1: 没有找到相同的mac
// 0--63 找到了相同的mac
int dev_find_list(Device_t dev)
{
	uint8_t i,j,num;
	
	for(i=0; i<SUPPORT_DEV_ONLINE_MAX; i++)
	{
		if((Dev_list[i].type<=3)&&(Dev_list[i].type!=0))  // 1 2 3
		{
			if(Dev_list[i].type==3)
				num = 32;
			else
				num  = 6;
			for(j=0; j<num; j++)
			{
				if(dev.mac[j]!=Dev_list[i].mac[j])
					break;
			}
			if(j==6)
				return i;
		}
	}
	return -1;
}

//-1 没有空的MAC
//0--63 返回空的list
int dev_find_Null_list(void)
{
	uint8_t i;
	
	for(i=0; i<SUPPORT_DEV_ONLINE_MAX; i++)
	{
		if((Dev_list[i].type>3)||(Dev_list[i].type==0))  
			return i;
	}
	return -1;
}

int dev_insert_list(Device_t dev)
{
	int a;
	
	if(!((dev.type==1)||(dev.type==2)||(dev.type==3)))
		return -1;
	
	if(dev_find_list(dev)==-1)
	{
		a = dev_find_Null_list();
		if((a>=0)&&(a<=SUPPORT_DEV_ONLINE_MAX-1))
		{
			memcpy(&Dev_list[a], &dev, sizeof( Device_t));
			return a;
		}
			
	}
	else
		dev_refresh_list(dev);
	return -1;
}
int dev_refresh_list(Device_t dev)
{
	int n;
	
	n = dev_find_list(dev);
	if(n!=-1)
	{
		Dev_list[n].time = GetLocalTime();
		return n;
	}
//	else
//		dev_insert_list(dev);
	return 1;
		
}

int addr1Toaddr2len(char *addr1, char *addr2)
{
	char *p1, *p2;
	int len = 0;
	
	p1 = addr1;
	p2 = addr2;
	
	if(p1>p2)
		return -1;
	while(p1!=p2)
	{
		len++;
		p1++;
	}
	return len;
}
