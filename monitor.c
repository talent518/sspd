#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <dirent.h>

#include "top.h"

#define LINES 20
#define NPROC 100

#define cpu_mem_head(c, m) if(hasCpu) {printf(c);} if(hasCpu && hasMem) {printf("|");} if(hasMem) {printf(m);} printf("\n")

char *fsize(unsigned long int size) {
	static char buf[20];
	static char units[5] = "KMGT";
	unsigned int unit;

	if(!size) {
		return "0K";
	}

	unit = (int)(log(size)/log(1024));
	if (unit > 3) {
		unit=3;
	}

	sprintf(buf, "%.2f%c", size/pow(1024,unit), units[unit]);

	return buf;
}

int getcomm(char *pid, char *comm) {
	static char buff[128] = "";
	static char fname[267] = "";
	FILE *fp;
	char *ptr;
	int len;

	snprintf(fname, sizeof(fname), "/proc/%s/comm", pid);

	fp = fopen(fname, "r");
	if(!fp) {
		return 0;
	}

	memset(buff, 0, sizeof(buff));
	len = fread(buff, 1, sizeof(buff) - 1, fp);

	fclose(fp);

	if(len <= 0) {
		return 0;
	}

	ptr = buff + len - 1;
	while(buff <= ptr && (*ptr == '\r' || *ptr == '\n' || *ptr == ' ')) {
		*ptr-- = '\0';
	}

	return !strcmp(buff, comm);
}

char procArgStr[NPROC][1024];

int procarg(char *comm, int nproc, int *pid, process_t *proc, unsigned int *pall) {
	static char fname[64] = "";
	int n, len, i;

	if(comm) {
		DIR *dp;
		struct dirent *dt;
		register char *p;

		nproc = 0;
		dp = opendir("/proc");
		while((dt = readdir(dp)) != NULL) {
			p = dt->d_name;
			while(*p && *p >= '0' && *p <= '9') p++;
			if(*p == '\0') {
				if(getcomm(dt->d_name, comm)) {
					pid[nproc++] = atoi(dt->d_name);
				}
			}
		}
		closedir(dp);
	}

	for(n=0; n<nproc; n++) {
		int len = snprintf(fname, sizeof(fname), "/proc/%d/cmdline", pid[n]);
		FILE *fp = fopen(fname, "r");
		if(!fp) {
			pid[n] = 0;
			continue;
		}
		len = fread(procArgStr[n], 1, sizeof(procArgStr[0]), fp);
		fclose(fp);

		procArgStr[n][len-1] = '\0';

		for(i=0; i<len-1; i++) {
			if(procArgStr[n][i] == '\0') {
				procArgStr[n][i] = ' ';
			}
		}

		if(!getprocessinfo(pid[n], &proc[n])) {
			pid[n] = 0;
			continue;
		}

		pall[n] = proc[n].utime + proc[n].stime + proc[n].cutime + proc[n].cstime;
	}

	return nproc;
}

int main(int argc, char *argv[]){
	cpu_t cpu, cpu2;
	mem_t mem;
	int pid[NPROC];
	process_t proc[NPROC], proc2[NPROC];

	unsigned int all, all2, pall[NPROC], pall2[NPROC], i, delay = 1;

	double total;
	long int realUsed;
	char hasCpu = 1, hasMem = 1;
	int nproc = 0, n;
	char *comm = NULL;

	for(i=1; i<argc; i++) {
		switch(argv[i][0]) {
			case '-':
				switch(argv[i][1]) {
					case 'c':
						hasMem = 0;
						hasCpu = 1;
						break;
					case 'm':
						hasMem = 1;
						hasCpu = 0;
						break;
					case 'P':
						if(i+1 < argc && nproc == 0) {
							hasCpu = 0;
							hasMem = 0;
							comm = argv[i+1];
							i++;
							break;
						}
					case 'p':
						if(i+1 < argc && comm == NULL) {
							hasCpu = 0;
							hasMem = 0;
							if(nproc<NPROC) {
								pid[nproc++] = atoi(argv[i+1]);
							}
							i++;
							break;
						}
					default:
						printf("Usage: %s [ -c | -m | -P <comm> | -p <pid> | -h | -? ] [ delay]\n"
								"  -c        Cpu info\n"
								"  -m        Memory info\n"
								"  -P <comm>  Process comm\n"
								"  -p <pid>  Process info(Multiple)\n"
								"  -h,-?     This help\n", argv[0]);
						return 0;
				}
				break;
			default:
				delay = atoi(argv[1]);
				if(delay <= 0) {
					delay = 1;
				}
				break;
		}
	}

	i = LINES;

	if(hasCpu) {
		getcpu(&cpu);

		all = cpu.user + cpu.nice + cpu.system + cpu.idle + cpu.iowait + cpu.irq + cpu.softirq + cpu.stolen + cpu.guest;
	}

	nproc = procarg(comm, nproc, pid, proc, pall);

	if(nproc>0 || comm) {
		getcpu(&cpu);
		all = cpu.user + cpu.nice + cpu.system + cpu.idle + cpu.iowait + cpu.irq + cpu.softirq + cpu.stolen + cpu.guest;
	}

	int nn, lines = 0;
	while(1) {
		if(lines++ % LINES == 0) {
			if(hasCpu && hasMem) {
				cpu_mem_head("------------------------------------------------------------", "------------------------------------------------------|-------------------");
				cpu_mem_head("                           CPU (%%)                          ", "                      Memory Size                     |  Real Memory (%%)  ");
			}

			if(hasCpu || hasMem) {
				cpu_mem_head("------------------------------------------------------------", "------------------------------------------------------|-------------------");
				cpu_mem_head(" User  Nice System   Idle IOWait   IRQ SoftIRQ Stolen  Guest", "MemTotal  MemFree   Cached  Buffers SwapTotal SwapFree|Memory Cached  Swap");
				cpu_mem_head("------------------------------------------------------------", "------------------------------------------------------|-------------------");
			}

			nproc = procarg(comm, nproc, pid, proc, pall);

			if(nproc > 0) {
				printf("--------------------------------------------------------------------------------------------------------------------\n");
				nn = nproc;
				for(n=0; n<nproc; n++) {
					if(pid[n]>0) {
						unsigned int mtime = proc[n].etime/60;
						unsigned int htime = mtime/60;
						unsigned int dtime = htime/24;
						printf("%d:\n  Run Time:(%lu seconds) ", pid[n], proc[n].etime);
						if(dtime>0) {
							printf("%d-", dtime);
						}
						printf("%02d:%02d:%02ld\n", htime%24, mtime%60, proc[n].etime%60);
						printf("  Command: %s\n", procArgStr[n]);
					} else {
						nn --;
					}
				}
				if(nn<=0 && comm == NULL) {
					return 0;
				}
				printf("--------------------------------------------------------------------------------------------------------------------\n");
				printf("        |                          Memory Size                                    |        |         CPU (%%)       \n");
				printf("   PID  |-------------------------------------------------------------------------|nThreads|------------------------\n");
				printf("        |    Size      RSS    Share     Text  Library Data+Stack    Dirty     Real|        |    User  Kernel   Total\n");
				printf("--------|-------------------------------------------------------------------------|--------|------------------------\n");
			}
			fflush(stdout);
			lines = 1;
		}

		sleep(delay);

		if(hasCpu) {
			getcpu(&cpu2);

			all2 = cpu2.user + cpu2.nice + cpu2.system + cpu2.idle + cpu2.iowait + cpu2.irq + cpu2.softirq + cpu2.stolen + cpu2.guest;

			total = (all2 - all) / 100.0;

			printf("%5.2f", (float)((double)(cpu2.user - cpu.user) / total));
			printf("%6.2f", (float)((double)(cpu2.nice - cpu.nice) / total));
			printf("%7.2f", (float)((double)(cpu2.system - cpu.system) / total));
			printf("%7.2f", (float)((double)(cpu2.idle - cpu.idle) / total));
			printf("%7.2f", (float)((double)(cpu2.iowait - cpu.iowait) / total));
			printf("%6.2f", (float)((double)(cpu2.irq - cpu.irq) / total));
			printf("%8.2f", (float)((double)(cpu2.softirq - cpu.softirq) / total));
			printf("%7.2f", (float)((double)(cpu2.stolen - cpu.stolen) / total));
			printf("%7.2f", (float)((double)(cpu2.guest - cpu.guest) / total));

			cpu = cpu2;
			all = all2;
		}

		if(hasCpu && hasMem) {
			printf("|");
		}

		if(hasMem) {
			getmem(&mem);
			printf("%8s", fsize(mem.total));
			printf("%9s", fsize(mem.free));
			printf("%9s", fsize(mem.cached));
			printf("%9s", fsize(mem.buffers));
			printf("%10s", fsize(mem.swapTotal));
			printf("%9s|", fsize(mem.swapFree));
			//printf("%6.2f", (float)((double)(mem.total - mem.free) * 100.0 / (double)mem.total)); // MemPercent
			printf("%6.2f", (float)((double)(realUsed = mem.total - mem.free - mem.cached - mem.buffers) * 100.0 / (double)mem.total)); // MemRealPercent
			printf("%7.2f", (float)((double)(mem.cached) * 100.0 / (double)mem.total)); // MemCachedPercent
			printf("%6.2f", (float)(mem.swapTotal ? (double)(mem.swapTotal - mem.swapFree) * 100.0 / (double)mem.swapTotal : 0.0)); // SwapPercent
		}

		if(hasCpu || hasMem) {
			printf("\n");
		}

		if(nproc>0) {
			getcpu(&cpu);

			all2 = cpu.user + cpu.nice + cpu.system + cpu.idle + cpu.iowait + cpu.irq + cpu.softirq + cpu.stolen + cpu.guest;
			total = (all2 - all) / 100.0;
			all = all2;
		}

		nn = nproc;
		for(n=0; n<nproc; n++) {
			if(pid[n] ==0 || !getprocessinfo(pid[n], &proc2[n])) {
				pid[n] = 0;
				nn--;
				continue;
			}

			pall2[n] = proc2[n].utime + proc2[n].stime + proc2[n].cutime + proc2[n].cstime;

			printf("%8d|", pid[n]);
			printf("%8s", fsize(proc2[n].size * 4));
			printf("%9s", fsize(proc2[n].resident * 4));
			printf("%9s", fsize(proc2[n].share * 4));
			printf("%9s", fsize(proc2[n].text * 4));
			printf("%9s", fsize(proc2[n].lib * 4));
			printf("%11s", fsize(proc2[n].data * 4));
			printf("%9s", fsize(proc2[n].dirty));
			printf("%9s|", fsize(proc2[n].rssFile));
			printf("%8d|", proc2[n].threads);

			double utime =  (double)(proc2[n].utime - proc[n].utime + proc2[n].cutime - proc[n].cutime) / total;
			double stime =  (double)(proc2[n].stime - proc[n].stime + proc2[n].cstime - proc[n].cstime) / total;
			printf("%8.2f", (float) utime);
			printf("%8.2f", (float) stime);
			printf("%8.2f\n", (float)(utime + stime));

			proc[n] = proc2[n];
		}
		if(nproc > 0 && nn <= 0 && comm == NULL) {
			return 0;
		} else if(nn > 1) {
			printf("--------------------------------------------------------------------------------------------------------------------\n");
		}

		fflush(stdout);
	}

	return 0;
}
