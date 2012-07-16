#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *gad(const char *argv0){
	char *bpath;
	bpath=(char *)malloc(255);
	strncpy(bpath,argv0,strrchr(argv0,'/')-argv0);
	if(*bpath=='.'){
		char *path,*q,*p,c;

		path=(char *)malloc(255);
		getcwd(path,255);

		printf("path : %s\n",path);
		printf("bpath : %s\n",bpath);

		if(*(bpath+1)=='/'){
			strcat(path,bpath+1);
		}else{
			strcat(path,"/");
			strcat(path,bpath);
			strcat(path,"\0");
		}

		printf("path:%s\n",path);

		while(p=strstr(path,"..")){
			q=p-1;
			do{
				q--;
			}while(q>path && *q!='/');
			q++;
			printf("before char '%c'\n",*q);
			p+=3;
			printf("after char '%c'\n",*p);
			while(q>path && q<path+255){
				if(*p!=0){
					printf("q:%d,p:%d;",q-path,p-path);
					printf("q:%c,p:%c\n",*q,*p);
					if(p==path+254){
						*p=0;
					}
				}
				*q=*p;
				q++;
				if(*p!=0){
					p++;
				}
			}
			printf("path:%s\n",path);
		}
		return path;
	}else{
		return bpath;
	}
}
