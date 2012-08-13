#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

typedef struct user{
	int sockfd;
	char host[15];
	uint16_t port;
	bool flag;
	struct user *prev;
	struct user *next;
} node;

extern int node_num;
extern node *head;
extern pthread_mutex_t node_mutex;

void construct();
node *find(int sockfd,bool is_port);
void insert(node *ptr);
void delete(node *ptr);
void destruct();
