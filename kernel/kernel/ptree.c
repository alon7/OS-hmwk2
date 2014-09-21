#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/prinfo.h>
#include "sched.h"

/**
 * sys_ptree - get the process tree
 * @buf: to store the processes tree
 * @nr: number of processes in the tree
 */
SYSCALL_DEFINE2(ptree, struct pinfo __user *, buf, int __user *, nr)
{
	read_lock(&tasklist_lock);

	read_unlock(&tasklist_lock);
}
