#ifndef __CPU_MEMORY_INFO_H
#define __CPU_MEMORY_INFO_H

typedef struct {
	unsigned long int user;
	unsigned long int nice;
	unsigned long int system;
	unsigned long int idle;
	unsigned long int iowait;
	unsigned long int irq;
	unsigned long int softirq;
	unsigned long int stolen;
	unsigned long int guest;
} cpu_t;

typedef struct {
	unsigned long int total; // MemTotal
	unsigned long int free; // MemFree
	unsigned long int cached; // Cached
	unsigned long int buffers; // Buffers
	unsigned long int locked; // Mlocked
	unsigned long int swapTotal; // SwapTotal
	unsigned long int swapFree; // SwapFree
	unsigned long int shared; // Shmem
} mem_t;

void getcpu(cpu_t *cpu);
void getmem(mem_t *mem);

typedef struct {
	// process memory
	long int size; // total program size (same as VmSize in /proc/[pid]/status)
	long int resident; // resident set size (same as VmRSS in /proc/[pid]/status)
	long int share; // shared pages (i.e., backed by a file)
	long int text; // text (code)
	long int lib; // library (unused in Linux 2.6)
	long int data; // data + stack
	long int dirty; // dirty KB(unused in Linux 2.6)
	long int rssFile; // resident - dirty KB

	// process cpu
	unsigned int threads; // Number of threads in this process (since Linux 2.6)
	long int utime; // Amount  of  time that this process has been scheduled in user mode, measured in clock ticks
	long int stime; // Amount of time that this process has been scheduled in kernel mode, measured in clock ticks
	long int cutime; // Amount of time that this process's waited-for children have been scheduled in user mode, measured in clock ticks
	long int cstime; // Amount of time that this process's waited-for children have been scheduled in kernel mode, measured in clock ticks
	
	unsigned long int etime; // runned time for seconds
} process_t;

int getprocessinfo(int pid, process_t *proc);

#endif // __CPU_MEMORY_INFO_H
