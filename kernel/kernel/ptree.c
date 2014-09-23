#include <linux/cred.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/prinfo.h>
#include <linux/threads.h>
#include <linux/syscalls.h>
#include <linux/init_task.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <asm/io.h>
/**
 * struct prinfo {
 * 	pid_t parent_pid;		 process id of parent 
 * 	pid_t pid;			 process id 
 * 	pid_t first_child_pid;  	 pid of youngest child 
 * 	pid_t next_sibling_pid;  	 pid of older sibling 
 * 	long state;			 current state of process 
 * 	long uid;			 user id of process owner 
 * 	char comm[64];			 name of program executed 
 * };
 */

void dfs_prinfo_copy(struct task_struct *head, struct prinfo *kernel_buf, int *copy_count, int *pr_count, int space_count);

/**
 * sys_ptree - get the process tree
 * @buf: to store the processes tree
 * @nr: number of processes in the tree
 */
SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
	int copy_count = 0;
	int pr_count = 0;
	int space_count = 0;
	struct task_struct *p;
	struct prinfo *kernel_buf;
	if (nr == NULL) {
		return -EINVAL;
	} 
	if (access_ok(VERIFY_WRITE, nr, sizeof(int)) == 0) {
		return -EFAULT;
	}
	if (copy_from_user(&space_count, nr, sizeof(int)) != 0) {
		return -EINVAL;
	}
	if (buf == NULL) {
		return -EINVAL;
	} 
	if (access_ok(VERIFY_WRITE, buf, space_count * sizeof(struct prinfo)) == 0) {
		return -EFAULT;
	}
	kernel_buf = kmalloc(space_count * sizeof(struct prinfo), __GFP_WAIT);
	read_lock(&tasklist_lock);
	p = &init_task;
	/* find root */
	while(p->real_parent != NULL && p->real_parent->pid != 0 && p->real_parent != p) {
		p = p->real_parent;
	}
	/* dfs */
	while(1) {
		dfs_prinfo_copy(p, kernel_buf, &copy_count, &pr_count, space_count);
		if (p->children.next != &p->children) {
			p = list_entry(p->children.next, struct task_struct, sibling);
			continue;
		}
		if (p->sibling.next != NULL && p->sibling.next != &p->sibling 
			&& p->sibling.next != &p->real_parent->children) {
			p = list_entry(p->sibling.next, struct task_struct, sibling);
			continue;
		}
		while(1) {
			if(p->real_parent != NULL && p->real_parent->pid != 0 && p->real_parent != p) {
				p = p->real_parent;
			}
			else {
				p = NULL;
				break;
			}
			if (p->sibling.next != NULL && p->sibling.next != &p->sibling 
				&& p->sibling.next != &p->real_parent->children) {
				p = list_entry(p->sibling.next, struct task_struct, sibling);
				break;
			}
		}
		if (p == NULL)
			break;
	}
	read_unlock(&tasklist_lock);
	if (copy_to_user(buf, kernel_buf, sizeof(struct prinfo) * copy_count) != 0) {
		return -EINVAL;
	}
	if (copy_to_user(nr, &copy_count, sizeof(int)) != 0) {
		return -EINVAL;
	}	
	kfree(kernel_buf);
	return pr_count;
}

void dfs_prinfo_copy(struct task_struct *head, struct prinfo *kernel_buf, int *copy_count, int *pr_count, int space_count)
{
	struct task_struct *p = head;
	int i;
	(*pr_count)++;
	if (*copy_count < space_count) {
		kernel_buf[*copy_count].parent_pid = p->real_parent->pid;
		kernel_buf[*copy_count].pid = p->pid;
		if (p->children.next == &p->children)
			kernel_buf[*copy_count].first_child_pid = 0;
		else
			kernel_buf[*copy_count].first_child_pid = list_entry(p->children.next, struct task_struct, sibling)->pid;
		if (p->sibling.next == NULL || p->sibling.next == &p->sibling 
			|| p->sibling.next == &p->real_parent->children)
			kernel_buf[*copy_count].next_sibling_pid = 0;
		else
			kernel_buf[*copy_count].next_sibling_pid = list_entry(p->sibling.next, struct task_struct, sibling)->pid;	
		kernel_buf[*copy_count].state = p->state;
		kernel_buf[*copy_count].uid = p->cred->uid;
		for (i = 0; i < 64; ++i) {
			(kernel_buf[*copy_count].comm)[i] = (p->comm)[i];
			if ((p->comm)[i] == '\0')
				break;
		}
		(*copy_count)++;
	}
}
