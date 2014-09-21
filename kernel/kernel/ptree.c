#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/prinfo.h>
#include <linux/threads.h>
#include <linux/syscalls.h>

/**
 * sys_ptree - get the process tree
 * @buf: to store the processes tree
 * @nr: number of processes in the tree
 */
SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
	read_lock(&tasklist_lock);
	read_unlock(&tasklist_lock);
	return 0;
}
