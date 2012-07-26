#include "api.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <locale.h>

#include <sys/types.h>
#include <ctype.h>
#include <curses.h>
#include <signal.h>

char *apidfile;

void stop(int sid){
	unlink(apidfile);
	exit(0);
}

int main(int argc, char *argv[]){
	if(argc!=3){
		printf("%s <pidfile> <command>\n",argv[0]);
		return 1;
	}

	FILE *fp;
	int pid;
	char *pidfile;
	char *command;
	char *ps;

	apidfile=gad(argv[0]);
	strcat(apidfile,"/daemon.pid");

	pidfile=strdup(argv[1]);
	command=strdup(argv[2]);

	int i;
	for (i = 1; i < argc; i++) {
		memset(argv[i], 0, strlen(argv[i]));
	}

	fp=fopen(apidfile,"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		kill(pid,SIGTERM);
	}

	pid=fork();

	if(pid==-1){
		printf("fork() error!\n");
	}
	if(pid>0){
		FILE *fp;
		fp=fopen(apidfile,"w+");
		if(fp==NULL){
			printf("file '%s' open fail.\n",apidfile);
		}else{
			fprintf(fp,"%d",pid);
			fclose(fp);
		}
		return 0;
	}

	signal(SIGHUP,stop);
	signal(SIGTERM,stop);
	signal(SIGINT,stop);
	signal(SIGKILL,stop);
	signal(SIGSTOP,stop);
	signal(SIGTSTP,stop);

	while(true){
		fp=fopen(pidfile,"r+");
		if(fp!=NULL){
			fscanf(fp,"%d",&pid);
			fclose(fp);

			ps=(char *)malloc(255);
			sprintf(ps,"ps -o 'pcpu,rsz,vsz,stat,start,time' -p %d",pid);

			if(pid!=getsid(pid)){
				system("echo \"start time : `date`\"");
				system(command);
				continue;
			}
			i=0;
			while(pid==getsid(pid)){
				if(i>10){
					i=0;
					system(ps);
					system("echo");
				}
				i++;
				sleep(1);
			}
			free(ps);
			system("echo \"stop time : `date`\"");
		}else{
			sleep(1);
		}
	}
	return 0;
}
