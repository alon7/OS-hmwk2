/*
	main.c
	Created by Wendan Kang on 9/21/14.
	Copyright (c) 2014 Wendan Kang. All rights reserved.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "kernel/include/linux/prinfo.h"
struct linknode {
	struct prinfo task;
	int top;
	struct linknode *next;
} LiStack;
LiStack *s;
void InitStack(void)
{
	s = (LiStack *)malloc(sizeof(LiStack));
	s->top = -1;
	s->next = NULL;
}
void Push(struct prinfo element)
{
	LiStack *p;

	p = (LiStack *)malloc(sizeof(LiStack));
	p->task = element;
	p->next = s->next;
	s->next = p;
	s->top++;
}
int Pop(void)
{
	LiStack *p;

	if (s->top == -1)
		return 0;
	p = s->next;
	s->next = p->next;
	s->top--;
	free(p);
	return 1;
}
void ClearStack(void)
{
	LiStack *p = s;
	LiStack *q = s->next;

	while (q != NULL) {
		free(p);
		p = q;
		q = p->next;
	}
	free(p);
}
int main(int argc, const char *argv[])
{
	struct prinfo *buf;
	int *nr;
	int i, j;

	nr = malloc(sizeof(int));
	*nr = 500;
	buf = (struct prinfo *)malloc(sizeof(struct prinfo) * 500);
	syscall(223, buf, nr);
	InitStack();
	for (i = 0; i < *nr; i++) {
		Push(*(buf+i));
		for (j = 0; j < s->top; j++)
			printf("\t");
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
		} else if (s->next->task.first_child_pid == (buf + i + 1)->pid)
			continue;
		while (s->next->task.next_sibling_pid != (buf + i + 1)->pid &&
			s->top != -1)
			Pop();
		Pop();
	}
	free(nr);
	nr = NULL;
	free(buf);
	buf = NULL;
	ClearStack();
	return 0;
}
