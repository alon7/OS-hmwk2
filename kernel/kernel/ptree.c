#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/prinfo.h>
#include "sched.h"

int ptree(struct prinfo *buf, int *nr)
{
	
}

/**
 * sys_ptree - get the process tree
 * @buf: to store the processes tree
 * @nr: number of processes in the tree
 */
SYSCALL_DEFINE2(ptree, struct pinfo __user *, buf, int __user *, nr)
{

}
