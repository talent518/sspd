#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "server.h"
#include "node.h"

unsigned int node_num=0;
node *head=NULL;

static int tstack=-1;
static unsigned int *istacks;

const unsigned int col_num=128;
unsigned int row_num;
node ***gnodes;

pthread_mutex_t mx_reader,mx_writer;
static unsigned int readers;

void begin_read_node(){
	if(head==NULL){
		return;
	}
	pthread_mutex_lock(&mx_reader);
	if ((++(readers)) == 1) {
		pthread_mutex_lock(&mx_writer);
	}
	pthread_mutex_unlock(&mx_reader);
}
void end_read_node(){
	if(head==NULL){
		return;
	}
	pthread_mutex_lock(&mx_reader);
	if ((--(readers)) == 0) {
		pthread_mutex_unlock(&mx_writer);
	}
	pthread_mutex_unlock(&mx_reader);
}
void begin_write_node(){
	if(head==NULL){
		return;
	}
	pthread_mutex_lock(&mx_writer);
}
void end_write_node(){
	if(head==NULL){
		return;
	}
	pthread_mutex_unlock(&mx_writer);
}

void calc_xy_by_index(unsigned int index,int *x,int *y){
	*x=index/row_num;
	*y=index%row_num;
}

unsigned int num_node(){
	unsigned int ret;
	BEGIN_READ_NODE{
		ret=node_num;
	}END_READ_NODE;
	return ret;
}

void attach_node(){
	if(head!=NULL){
		return;
	}
	readers=0;

	pthread_mutex_init(&mx_reader, NULL);
	pthread_mutex_init(&mx_writer, NULL);

	head=(node *)malloc(sizeof(node));
	head->index=0;
	head->prev=head;
	head->next=head;

	istacks=(unsigned int *)calloc(sizeof(unsigned int),ssp_maxclients);
	row_num=ssp_maxclients/col_num+(ssp_maxclients%col_num?1:0);
	gnodes=(node ***)malloc(sizeof(node**)*row_num);
	int x,y;
	for(y=0;y<row_num;y++){
		gnodes[y]=(node **)malloc(sizeof(node*)*col_num);
		for(x=0;x<col_num;x++){
			gnodes[y][x]=NULL;
		}
	}
}

node *index_node(unsigned int index){
	node *ptr;

	BEGIN_READ_NODE{
		int x,y;
		calc_xy_by_index(index,&x,&y);
		ptr=gnodes[y][x];
	}END_READ_NODE;
	return ptr;
}

node *search_node(int sockfd,bool is_port){
	if(head==NULL){
		return;
	}
	node *ptr=NULL;
	BEGIN_READ_NODE{
		node *p=head;
		while(p->next!=head){
			p=p->next;
			if((is_port?p->port:p->sockfd)==sockfd){
				ptr=p;
				break;
			}
		}
	}END_READ_NODE;
	return ptr;
}

void insert_node(node *ptr){
	if(ptr==head || ptr==NULL){
		dprintf("\nInsert node (ptr can't NULL or head) error!\n");
		return;
	}
	BEGIN_WRITE_NODE{
		if(node_num<ssp_maxclients){
			node *hp=head->prev;

			hp->next=ptr;
			ptr->prev=hp;

			head->prev=ptr;
			ptr->next=head;

			node_num++;

			ptr->index=(tstack<0?node_num:istacks[tstack--]);

			int x,y;
			calc_xy_by_index(ptr->index-1,&x,&y);
			gnodes[y][x]=ptr;
		}else{
			ptr->prev=NULL;
			ptr->next=NULL;
			ptr->index=0;
		}
	}END_WRITE_NODE;
}

void remove_node(node *ptr){
	if(ptr==head || ptr==NULL){
		dprintf("\nRemove node (ptr can't NULL or head) error!\n");
		return;
	}
	BEGIN_WRITE_NODE{
		node *p=ptr->prev,*n=ptr->next;

		p->next=n;
		n->prev=p;

		int x,y;
		calc_xy_by_index(ptr->index-1,&x,&y);
		gnodes[y][x]=NULL;

		node_num--;

		if(node_num){
			istacks[++tstack]=ptr->index;
		}else{
			tstack=-1;
		}
		free(ptr);
	}END_WRITE_NODE;
}

void detach_node(){
	if(head==NULL){
		return;
	}
	BEGIN_WRITE_NODE{
		node *p=head;
		head->prev->next=NULL;
		while(p->next!=NULL){
			p=p->next;
			free(p->prev);
		}
		free(p);
		tstack=-1;
		free(istacks);
		int i;
		for(i<0;i<row_num;i++){
			free(gnodes[i]);
		}
		free(gnodes);
		head=NULL;
	}END_WRITE_NODE;
	pthread_mutex_destroy(&mx_reader);
	pthread_mutex_destroy(&mx_writer);
}
