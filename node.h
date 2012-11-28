#ifndef _NODE_H
#define _NODE_H

#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

typedef struct _node{
	pthread_t tid;
	int sockfd;
	char host[15];
	uint16_t port;
	bool reading;
	unsigned int index;
	struct _node *prev;
	struct _node *next;
} node;

extern const unsigned int col_num;
extern unsigned int row_num;
extern node ***gnodes;

extern node *head;

#define BEGIN_READ_NODE begin_read_node();
#define END_READ_NODE end_read_node();
#define BEGIN_WRITE_NODE begin_write_node();
#define END_WRITE_NODE end_write_node();

#define NODE_NUM num_node()

//读写锁
void begin_read_node();
void end_read_node();
void begin_write_node();
void end_write_node();

//接点个数，不包括head接点
unsigned int num_node();

//接点读
node* index_node(unsigned int index);
node* search_node(int sockfd,bool is_port);

//接点写
void attach_node();
void insert_node(node *ptr);
void remove_node(node *ptr);
void detach_node();

#endif