#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "node.h"

int node_num=0;
node *head=NULL;
pthread_mutex_t node_mutex;

void construct(){
	if(head!=NULL){
		return;
	}
	head=(node *)malloc(sizeof(node));
	head->prev=head;
	head->next=head;
    pthread_mutex_init(&node_mutex, NULL);
}

node *find(int sockfd,bool is_port){
	pthread_mutex_lock(&node_mutex);
	node *p,*ptr=NULL;
	p=head;
	while(p->next!=head){
		p=p->next;
		if((is_port?p->port:p->sockfd)==sockfd){
			ptr=p;
			break;
		}
	}
	pthread_mutex_unlock(&node_mutex);
	return ptr;
}

void insert(node *ptr){
	if(ptr==head || ptr==NULL){
		printf("\nInsert node (ptr can't NULL or head) error!\n");
		return;
	}
	pthread_mutex_lock(&node_mutex);
	node *hp=head->prev;

	hp->next=ptr;
	ptr->prev=hp;

	head->prev=ptr;
	ptr->next=head;

	node_num++;
	pthread_mutex_unlock(&node_mutex);
}

void delete(node *ptr){
	if(ptr==head || ptr==NULL){
		printf("\nDelete node (ptr can't NULL or head) error!\n");
		return;
	}
	pthread_mutex_lock(&node_mutex);
	node *p=ptr->prev,*n=ptr->next;

	p->next=n;
	n->prev=p;

	free(ptr);
	node_num--;
	pthread_mutex_unlock(&node_mutex);
}

void destruct(){
	if(head==NULL){
		return;
	}
	while(head->next!=head){
		delete(head->next);
	}
	while(head->prev!=head){
		delete(head->prev);
	}
	free(head);
    pthread_mutex_destroy(&node_mutex);
}
