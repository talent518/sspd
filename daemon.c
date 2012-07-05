#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#if HAVE_SYS_TIME_H
	#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
	#include <unistd.h>
#endif
#if HAVE_SIGNAL_H
	#include <signal.h>
#endif
#if HAVE_SETLOCALE
	#include <locale.h>
#endif

int main(int argc, char *argv[]){
	FILE *fp;
	int pid;
	char *pidfile;
	char *command;

	if(argc!=3){
		printf("%s <pidfile> <command>\n",argv[0]);
		return 1;
	}

	pidfile=argv[1];
	command=argv[2];

	pid=fork();

	if(pid==-1){
		printf("fork() error!\n");
	}
	if(pid>0){
		return 0;
	}

	while(true){
		fp=fopen(pidfile,"r+");
		if(fp!=NULL){
			fscanf(fp,"%d",&pid);
			fclose(fp);
			while(pid==getsid(pid)){
				usleep(500);
			}
			unlink(pidfile);
			system(command);
		}
		sleep(1);
	}
	return 0;
}
