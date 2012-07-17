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

		if(*(bpath+1)=='.'){
			strcat(path,"/");
			strcat(path,bpath);
			strcat(path,"\0");
		}else{
			strcat(path,bpath+1);
		}

		while(p=strstr(path,"..")){
			q=p-1;
			do{
				q--;
			}while(q>path && *q!='/');
			q++;
			p+=3;
			while(q>path && q<path+255){
				if(*p!=0){
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
		}
		return path;
	}else{
		return bpath;
	}
}
