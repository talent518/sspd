#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#include "config.h"
#include "php_ext.h"
#include "php_func.h"
#include "ssp.h"
#include "data.h"
#include "api.h"
#include "ssp_event.h"

unsigned int ssp_backlog = 1024;

char *ssp_host = "0.0.0.0";
short int ssp_port = 8083;
char *ssp_pidfile = "/var/run/ssp.pid";
char *ssp_infile = "/dev/urandom";
char *ssp_outfile = "/var/run/ssp.out";
char *ssp_errfile = "/var/run/ssp.err";

char *ssp_user = "daemon";

int ssp_maxclients = 1000;
int ssp_maxrecvs = 2 * 1024 * 1024;

static zend_bool ssp_dup2() {
	int infd, outfd, errfd;

	infd = open(ssp_infile, O_RDONLY);
	if(infd < 0) {
		php_error(E_ERROR, "Error opening file \"%s\": errno = %d, errstr = \"%s\"\n", ssp_infile, errno, strerror(errno));
		return FAILURE;
	}

	outfd = open(ssp_outfile, O_CREAT|O_WRONLY|O_APPEND, 0755);
	if(outfd < 0) {
		php_error(E_ERROR, "Error opening file \"%s\": errno = %d, errstr = \"%s\"\n", ssp_outfile, errno, strerror(errno));
		return FAILURE;
	}

	errfd = open(ssp_errfile, O_CREAT|O_WRONLY|O_APPEND, 0755);
	if(errfd < 0) {
		close(outfd);
		php_error(E_ERROR, "Error opening file \"%s\": errno = %d, errstr = \"%s\"\n", ssp_errfile, errno, strerror(errno));
		return FAILURE;
	}

	dup2(infd, STDIN_FILENO);
	dup2(outfd, STDOUT_FILENO);
	dup2(errfd, STDERR_FILENO);

	close(infd);
	close(outfd);
	close(errfd);

	return SUCCESS;
}

void signal_handler(int sig) {
}

void server_start() {
#if 1
	int pid = fork();

	if (pid == -1) {
		perror("fork");
		return;
	}
	if (pid > 0) {
		signal(SIGINT, signal_handler);
		sleep(-1);
		return;
	}

	struct passwd *pwnam;
	pwnam = getpwnam(ssp_user);

	if (!pwnam) {
		perror("getpwnam");
		return;
	}

	setuid(pwnam->pw_uid);
	setgid(pwnam->pw_gid);

	if (setsid() < 0) perror("setsid");
#endif

	struct sockaddr_in sin;
	int listen_fd;
	int ret;

	int i = 19, cols = tput_cols();

	printf("Starting SSP server");
	strnprint(".", cols - i - 9);

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		printf("\033[31m[Failed]\033[m\n");
		perror("socket");
		return;
	}

	socket_set_listen(listen_fd);

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(ssp_host);
	sin.sin_port = htons(ssp_port);

	ret = bind(listen_fd, (struct sockaddr *) &sin, sizeof(sin));
	if (ret < 0) {
		printf("\033[31m[Failed]\033[m\n");
		perror("bind");
		close(listen_fd);
		return;
	}

	ret = listen(listen_fd, ssp_backlog);
	if (ret < 0) {
		printf("\033[31m[Failed]\033[m\n");
		perror("listen");
		close(listen_fd);
		return;
	}

	printf("\033[32m[Succeed]\033[m\n");

	FILE *fp;
	if ((fp = fopen(ssp_pidfile, "w+")) == NULL) {
		perror("fopen");
	} else {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	ssp_dup2();

	loop_event(listen_fd);

	close(listen_fd);
	if(unlink(ssp_pidfile) < 0) perror("unlink");
}

void server_stop() {
	FILE *fp;
	int pid, i = 19, cols = tput_cols();

	printf("Stopping SSP server");

	fp = fopen(ssp_pidfile, "r+");
	if (fp != NULL) {
		fscanf(fp, "%d", &pid);
		fclose(fp);
		if (pid == getsid(pid)) {
			kill(pid, SIGINT);
			while (pid == getsid(pid)) {
				printf(".");
				i++;
				sleep(1);
				if (cols - i - 9 == 0) {
					kill(pid, SIGKILL);
					break;
				}
			}
			strnprint(".", cols - i - 9);
			printf("\033[32m[Succeed]\033[m\n");
			return;
		}
	}
	strnprint(".", cols - i - 9);
	printf("\033[31m[stopped]\033[m\n");
}

void server_status() {
	FILE *fp;
	int pid, i = 17, cols = tput_cols();

	printf("SSP server status");
	strnprint(".", cols - i - 9);

	fp = fopen(ssp_pidfile, "r+");
	if (fp != NULL) {
		fscanf(fp, "%d", &pid);
		fclose(fp);
		if (pid == getsid(pid)) {
			printf("\033[32m[Running]\033[m\n");
			return;
		}
		unlink(ssp_pidfile);
	}
	printf("\033[31m[stopped]\033[m\n");
}

void server_bench() {
	loop_event(-1);
}
