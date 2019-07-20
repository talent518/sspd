#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

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

int main(int argc, char *argv[]){
	cpu_t cpu, cpu2;
	mem_t mem;
	int pid[NPROC];
	process_t proc[NPROC], proc2[NPROC];

	unsigned long int all, all2, i, delay = 1;

	double total;
	long int realUsed;
	char hasCpu = 1, hasMem = 1;
	int nproc = 0, n;

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
					case 'p':
						if(i+1 < argc) {
							hasCpu = 0;
							hasMem = 0;
							if(nproc<NPROC) {
								pid[nproc++] = atoi(argv[i+1]);
							}
						}
						break;
					case 'h':
					case '?':
						printf("Usage: %s [ -c | -m | -p <pid> | -h | -? ] [ delay]\n"
								"  -c        Cpu info\n"
								"  -m        Memory info\n"
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

	char procArgStr[NPROC][106];
	char fname[64] = "";
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
	}

	if(nproc>0) {
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

			if(nproc > 0) {
				if(lines == 1)
					printf("--------------------------------------------------------------------------------------------------------------------\n");
				nn = nproc;
				for(n=0; n<nproc; n++) {
					if(pid[n]>0) {
						unsigned int mtime = proc[n].etime/60;
						unsigned int htime = mtime/60;
						unsigned int dtime = htime/24;
						printf("%d:\n  Run Time:(%llu seconds) ", pid[n], proc[n].etime);
						if(dtime>0) {
							printf("%d-", dtime);
						}
						printf("%02d:%02d:%02d\n", htime%24, mtime%60, proc[n].etime%60);
						printf("  Command: %s\n", procArgStr[n]);
					} else {
						nn --;
					}
				}
				if(nn<=0) {
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

			printf("%5.2f", (double)(cpu2.user - cpu.user) / total);
			printf("%6.2f", (double)(cpu2.nice - cpu.nice) / total);
			printf("%7.2f", (double)(cpu2.system - cpu.system) / total);
			printf("%7.2f", (double)(cpu2.idle - cpu.idle) / total);
			printf("%7.2f", (double)(cpu2.iowait - cpu.iowait) / total);
			printf("%6.2f", (double)(cpu2.irq - cpu.irq) / total);
			printf("%8.2f", (double)(cpu2.softirq - cpu.softirq) / total);
			printf("%7.2f", (double)(cpu2.stolen - cpu.stolen) / total);
			printf("%7.2f", (double)(cpu2.guest - cpu.guest) / total);

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
		if(nproc>0 && nn<=0) {
			return 0;
		} else if(nn>1) {
			printf("--------------------------------------------------------------------------------------------------------------------\n");
		}

		fflush(stdout);
	}

	return 0;
}
