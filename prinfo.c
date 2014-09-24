//
//  main.c
//  testPrinfo
//
//  Created by Wendan Kang on 9/21/14.
//  Copyright (c) 2014 Wendan Kang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "kernel/include/linux/prinfo.h"
struct prinfo;
/*
struct prinfo{
	pid_t parent_pid;
	pid_t pid;
	pid_t first_child_pid;
   	pid_t next_sibling_pid;
   	long state;
   	long uid;
   	char comm[64];
};
*/

 
typedef struct linknode{
	struct prinfo task;
	int top;
	struct linknode *next;
}LiStack;
LiStack *s;
void InitStack(){
	s = (LiStack *)malloc(sizeof(LiStack));
	s->top = -1;
   	s->next = NULL;
}
void Push(struct prinfo element){
   	LiStack *p;
   	p = (LiStack *)malloc(sizeof(LiStack));
   	p->task = element;
   	p->next = s->next;
   	s->next = p;
   	s->top++;
}
int Pop(){
   	LiStack *p;
   	if(s->top == -1)
       		return 0;
   	p = s->next;
   	s->next = p->next;
   	s->top--;
   	free(p);
   	return 1;
}
int main(int argc, const char * argv[])
{
   	struct prinfo *buf;
   	int *nr;
   	nr = malloc(sizeof(int));
   	int taskNum;
   	buf = (struct prinfo*)malloc(sizeof(struct prinfo) * 500);
/*
   	buf->pid = 0;
   	buf->next_sibling_pid = 0;
   	buf->first_child_pid = 1;
    	(buf + 1)->comm = "init";
   	(buf + 1)->pid = 1;
   	(buf + 1)->next_sibling_pid = 2;
   	(buf + 1)->first_child_pid = 31;
   	(buf + 2)->comm = "ueventd";
   	(buf + 2)->pid = 31;
   	(buf + 2)->next_sibling_pid = 42;
   	(buf + 2)->first_child_pid = 0;
   	(buf + 3)->comm = "";
   	(buf + 3)->pid = 42;
   	(buf + 3)->next_sibling_pid = 43;
   	(buf + 3)->first_child_pid = 31;
   	(buf + 4)->pid = 7;
   	(buf + 4)->next_sibling_pid = 8;
   	(buf + 5)->pid = 2;
   	(buf + 5)->next_sibling_pid = 3;
   	(buf + 5)->first_child_pid = 8;
   	(buf + 6)->pid = 8;
   	(buf + 6)->next_sibling_pid = 9;
   	(buf + 6)->first_child_pid = 13;
   	(buf + 7)->pid = 13;
    	(buf + 7)->next_sibling_pid = 14;
   	(buf + 8)->pid = 14;
   	(buf + 8)->next_sibling_pid = 15;
   	(buf + 9)->pid = 9;
   	(buf + 9)->next_sibling_pid = 10;
   	(buf + 9)->first_child_pid = 15;
   	(buf + 10)->pid = 15;
   	(buf + 10)->next_sibling_pid = 0; //attention
   	(buf + 11)->pid = 3;
   	(buf + 11)->next_sibling_pid = 4;
   	(buf + 11)->first_child_pid = 10;
   	(buf + 12)->pid = 10;
   	(buf + 12)->next_sibling_pid = 11;
   	(buf + 13)->pid = 4;
   	(buf + 13)->next_sibling_pid = 0; //attention
   	(buf + 13)->first_child_pid = 11;
   	(buf + 14)->pid = 11;
   	(buf + 14)->next_sibling_pid = 12;
   	(buf + 15)->pid = 12;
   	(buf + 15)->next_sibling_pid = 0;
 */  
   	taskNum = syscall(223, buf, nr);
//	printf("=====NR: %d======\n", *nr);    
   	InitStack();
   	int i, j;
   	for (i = 0; i < taskNum; i++) {
       		Push(*(buf+i));
        	for (j = 0; j < s->top; j++) {
            		printf("\t");
        	}	
        	printf("%s,%d,%ld,%d,%d,%d,%ld\n",
               		(buf + i)->comm,
               		(buf + i)->pid,
               		(buf + i)->state,
               		(buf + i)->parent_pid,
               		(buf + i)->first_child_pid,
               		(buf + i)->next_sibling_pid,
               		(buf + i)->uid);

        	if (s->next->task.next_sibling_pid == (buf + i + 1)->pid) {
            		Pop();
            		continue;
        	}else if (s->next->task.first_child_pid == (buf + i + 1)->pid){
            		continue;
        	}
        	while (s->next->task.next_sibling_pid != (buf + i + 1)->pid && s->top != -1) {
            		Pop();
        	}
        	Pop();
    	}
    
   	return 0;
}


