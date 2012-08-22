#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

typedef struct user{
	pthread_t tid;
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

 void attach_node();
node* search_node(int sockfd,bool is_port);
 void insert_node(node *ptr);
 void remove_node(node *ptr);
 void detach_node();
