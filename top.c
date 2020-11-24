#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "top.h"

void getcpu(cpu_t *cpu) {
	static char buff[128];
	static char strcpu[8];
	FILE *fp;
	
	memset(cpu, 0, sizeof(cpu_t));

	fp = fopen("/proc/stat", "r");

	memset(buff, 0, sizeof(buff));
	fgets(buff, sizeof(buff) - 1, fp);

	fclose(fp);
	
	sscanf(buff, "%s%ld%ld%ld%ld%ld%ld%ld%ld%ld", strcpu, &cpu->user, &cpu->nice, &cpu->system, &cpu->idle, &cpu->iowait, &cpu->irq, &cpu->softirq, &cpu->stolen, &cpu->guest);
}

void getmem(mem_t *mem) {
	static char buff[2048] = "";
	static char key[20] = "";
	static long int val = 0;
	FILE *fp;
	char *ptr;
	int i = 0377;
	
	memset(mem, 0, sizeof(mem_t));
	
	fp = fopen("/proc/meminfo", "r");

	memset(buff, 0, sizeof(buff));
	fread(buff, sizeof(buff) - 1, 1, fp);

	fclose(fp);
	
	ptr = buff;
	while(ptr && sscanf(ptr, "%[^:]: %ld", key, &val)) {
		// printf("%s => %d\n", key, val);

		if((i & 01) && !strcmp(key, "MemTotal")) {
			mem->total = val;
			i ^= 01;
			continue;
		}
		if((i & 02) && !strcmp(key, "MemFree")) {
			mem->free = val;
			i ^= 02;
			continue;
		}
		if((i & 04) && !strcmp(key, "Buffers")) {
			mem->buffers = val;
			i ^= 04;
			continue;
		}
		if((i & 010) && !strcmp(key, "Cached")) {
			mem->cached = val;
			i ^= 010;
			continue;
		}
		if((i & 020) && !strcmp(key, "Mlocked")) {
			mem->locked = val;
			i ^= 020;
			continue;
		}
		if((i & 040) && !strcmp(key, "SwapTotal")) {
			mem->swapTotal = val;
			i ^= 040;
			continue;
		}
		if((i & 0100) && !strcmp(key, "SwapFree")) {
			mem->swapFree = val;
			i ^= 0100;
			continue;
		}
		if((i & 0200) && !strcmp(key, "Shmem")) {
			mem->shared = val;
			i ^= 0200;
			continue;
		}

		ptr = strchr(ptr, '\n');
		if(ptr) {
			ptr++;
		}
	}
}

//获取第N项开始的指针
const char* get_items(const char*buffer, unsigned int item) {
	const char *p =buffer;

	register int len = strlen(buffer);
	register int count = 0, i;

	for (i=0; i<len;i++){
		if (' ' == *p){
			count ++;
			if(count == item -1){
				p++;
				break;
			}
		}
		p++;
	}

	return p;
}

unsigned int getprocessdirtys(int pid) {
	static char buff[16*1024*1024] = "";
	static char fname[64] = "";
	static char key[64] = "";
	static long int val = 0;
	FILE *fp;
	char *ptr;
	
	snprintf(fname, sizeof(fname), "/proc/%d/smaps", pid);

	fp = fopen(fname, "r");
	if(!fp) {
		return 0;
	}

	memset(buff, 0, sizeof(buff));
	fread(buff, sizeof(buff) - 1, 1, fp);

	fclose(fp);
	
	ptr = buff;
	
	unsigned int dirtys = 0;
	while(ptr && sscanf(ptr, "%[^:]: %ld", key, &val)) {
		if(!strcmp(key, "Private_Dirty") || !strcmp(key, "Shared_Dirty")) {
			dirtys += val;
		}
		ptr = strchr(ptr, '\n');
		if(ptr) {
			ptr++;
		}
	}
	
	return dirtys;
}

int getprocessinfo(int pid, process_t *proc) {
	static char buff[1024] = "";
	static char fname[64] = "";
	static char key[20] = "";
	static long int val = 0;
	FILE *fp;
	char *ptr;

	snprintf(fname, sizeof(fname), "/proc/%d/statm", pid);

	fp = fopen(fname, "r");
	if(!fp) {
		return 0;
	}

	memset(buff, 0, sizeof(buff));
	fgets(buff, sizeof(buff) - 1, fp);

	fclose(fp);

	sscanf(buff, "%ld%ld%ld%ld%ld%ld%ld", &proc->size, &proc->resident, &proc->share, &proc->text, &proc->lib, &proc->data, &proc->dirty);

	snprintf(fname, sizeof(fname), "/proc/%d/stat", pid);

	fp = fopen(fname, "r");
	if(!fp) {
		return 0;
	}

	memset(buff, 0, sizeof(buff));
	fgets(buff, sizeof(buff) - 1, fp);

	fclose(fp);

	const char *q = get_items(buff, 14);
	sscanf(q, "%ld%ld%ld%ld", &proc->utime, &proc->stime, &proc->cutime, &proc->cstime);
	q = get_items(q, 7);
	sscanf(q, "%u", &proc->threads);
	q = get_items(q, 3);
	
	unsigned long int etime = 0;
	sscanf(q, "%lu", &etime);
	
	fp = fopen("/proc/uptime", "r");
	fgets(buff, sizeof(buff) - 1, fp);

	fclose(fp);
	
	unsigned long int uptime = 0;
	sscanf(buff, "%lu", &uptime);
	
	int tck = sysconf(_SC_CLK_TCK);
	proc->etime = uptime - etime / tck;

	snprintf(fname, sizeof(fname), "/proc/%d/status", pid);

	fp = fopen(fname, "r");
	if(!fp) {
		return 0;
	}

	memset(buff, 0, sizeof(buff));
	fread(buff, sizeof(buff) - 1, 1, fp);

	fclose(fp);

	ptr = buff;
	proc->dirty = 0;
	proc->rssFile = 0;
	while(ptr && sscanf(ptr, "%[^:]: %ld", key, &val)) {
		if(!strcmp(key, "RssFile")) {
			proc->rssFile = val;
			proc->dirty = proc->resident * 4 - val;
		}
		ptr = strchr(ptr, '\n');
		if(ptr) {
			ptr++;
		}
	}
	
	if(!proc->dirty || !proc->rssFile) {
		proc->dirty = getprocessdirtys(pid);
		proc->rssFile  = proc->resident * 4 - proc->dirty;
	}

	return 1;
}

