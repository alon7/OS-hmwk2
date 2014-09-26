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
 * sys_ptree - get the process tree
 * @buf: to store the processes tree
 * @nr: number of processes in the tree
 */
struct prinfo;

/* do dfs in a non-recursion way */
void dfs_prinfo_copy(struct task_struct *head, struct prinfo *kernel_buf,
	int *copy_count, int *pr_count, int space_count)
{
	struct task_struct *p = head;
	int i;
	(*pr_count)++;
	if (*copy_count < space_count) {
		kernel_buf[*copy_count].parent_pid = p->parent->pid;
		kernel_buf[*copy_count].pid = p->pid;
		if (p->children.next == &p->children)
			kernel_buf[*copy_count].first_child_pid = 0;
		else
			kernel_buf[*copy_count].first_child_pid =
		list_entry(p->children.next, struct task_struct, sibling)->pid;
		if (p->sibling.next == NULL || p->sibling.next == &p->sibling
			|| p->sibling.next == &p->parent->children)
			kernel_buf[*copy_count].next_sibling_pid = 0;
		else
			kernel_buf[*copy_count].next_sibling_pid =
		list_entry(p->sibling.next, struct task_struct, sibling)->pid;
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

void dfs(struct task_struct *p, struct prinfo *kernel_buf, int *copy_count,
	int *pr_count, int space_count)
{
	while (1) {
		dfs_prinfo_copy(p, kernel_buf, copy_count, pr_count,
			space_count);
		if (p->children.next != &p->children) {
			p = list_entry(p->children.next, struct task_struct,
				sibling);
			continue;
		}
		if (p->sibling.next != NULL && p->sibling.next != &p->sibling
			&& p->sibling.next != &p->parent->children) {
			p = list_entry(p->sibling.next, struct task_struct,
				sibling);
			continue;
		}
		while (1) {
			if (p->parent != NULL && p->parent->pid != 0
				&& p->parent != p) {
				p = p->parent;
			} else {
				p = NULL;
				break;
			}
			if (p->sibling.next != NULL
				&& p->sibling.next != &p->sibling
				&& p->sibling.next
					!= &p->parent->children){
				p = list_entry(p->sibling.next,
					struct task_struct, sibling);
				break;
			}
		}
		if (p == NULL)
			break;
	}
}

SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
	int copy_count = 0;
	int pr_count = 0;
	int space_count = 0;
	struct task_struct *p;
	struct prinfo *kernel_buf;

	if (nr == NULL)
		return -EINVAL;
	if (access_ok(VERIFY_WRITE, nr, sizeof(int)) == 0)
		return -EFAULT;
	if (copy_from_user(&space_count, nr, sizeof(int)) != 0)
		return -EINVAL;
	if (buf == NULL)
		return -EINVAL;
	if (access_ok(VERIFY_WRITE, buf, space_count * sizeof(struct prinfo))
		== 0)
		return -EFAULT;
	kernel_buf = kmalloc_array(space_count, sizeof(struct prinfo),
		__GFP_WAIT);
	if (kernel_buf == NULL)
		return -EFAULT;
	p = &init_task;
	/* find root */
	while (p->parent != NULL && p->parent->pid != 0
		&& p->parent != p)
		p = p->parent;
	/* do dfs in a non-recursion way */
	read_lock(&tasklist_lock);
	dfs(p, kernel_buf, &copy_count, &pr_count, space_count);
	read_unlock(&tasklist_lock);

	if (copy_to_user(buf, kernel_buf, sizeof(struct prinfo) * copy_count)
		!= 0)
		return -EINVAL;
	if (copy_to_user(nr, &copy_count, sizeof(int)) != 0)
		return -EINVAL;
	kfree(kernel_buf);
	kernel_buf = NULL;
	return pr_count;
}
