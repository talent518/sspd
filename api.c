#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

#define MICRO_IN_SEC 1000000.00

double microtime()
{
	struct timeval tp = {0};

	if (gettimeofday(&tp, NULL)) {
		return 0;
	}

	return (double)(tp.tv_sec + tp.tv_usec / MICRO_IN_SEC);
}

char *fsize(int size)
{
	char units[5][3]={"B","KB","MB","GB","TB"};
	char buf[10];
	int unit=(int)(log(size)/log(1024));

	if (unit>4)
	{
		unit=4;
	}

	sprintf(buf, "%.3f%s", size/pow(1024,unit), units[unit]);

	return strdup(buf);
}

char *gad(const char *argv0)
{
	char *bpath;
	bpath=(char *)malloc(255);
	strncpy(bpath,argv0,strrchr(argv0,'/')-argv0);
	if (*bpath=='.')
	{
		char *path,*q,*p,c;

		path=(char *)malloc(255);
		getcwd(path,255);

		if (*(bpath+1)=='.')
		{
			strcat(path,"/");
			strcat(path,bpath);
			strcat(path,"\0");
		}
		else
		{
			strcat(path,bpath+1);
		}

		while (p=strstr(path,".."))
		{
			q=p-1;
			do
			{
				q--;
			}
			while (q>path && *q!='/');
			q++;
			p+=3;
			while (q>path && q<path+255)
			{
				if (*p!=0)
				{
					if (p==path+254)
					{
						*p=0;
					}
				}
				*q=*p;
				q++;
				if (*p!=0)
				{
					p++;
				}
			}
		}
		return path;
	}
	else
	{
		return bpath;
	}
}

int execi(const char *cmd)
{
	FILE *fp;
	int ret=0;
	fp=popen(cmd,"r");
	if (fp!=NULL)
	{
		fscanf(fp,"%d",&ret);
		fclose(fp);
	}
	return ret;
}

char *str_repeat(const char *str,size_t str_len,size_t repeat)
{
	char *ret=(char *)malloc(str_len*repeat);
	size_t i;
	for (i=0;i<repeat;i++)
	{
		strncpy(ret+i*str_len,str,str_len);
	}
	return ret;
}

void strnprint(const char *str,size_t repeat)
{
	size_t i;
	for (i=0;i<repeat;i++)
	{
		printf(str);
	}
}