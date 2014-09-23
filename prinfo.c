#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "kernel/include/linux/prinfo.h"
struct prinfo;

int main(int argc, char **argv) {
	struct prinfo *buf = malloc(sizeof(struct prinfo) * 500);
	int nr = 500;
	int i;
	syscall(223, buf, &nr);
	for (i = 0; i < nr; ++i) {
		//printf(/* correct number of \t */);
		struct prinfo p = buf[i];
		printf("%s,%d,%ld,%d,%d,%d,%d\n", p.comm, p.pid, p.state,p.parent_pid, p.first_child_pid, p.next_sibling_pid, p.uid);
	}
	return 0;
}