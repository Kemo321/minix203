
/* Miscellaneous system calls.				Author: Kees J. Bot
 *								31 Mar 2000
 *
 * The entry points into this file are:
 *   do_reboot: kill all processes, then reboot system
 *   do_svrctl: memory manager control
 */

#include "mm.h"
#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include "mproc.h"
#include "param.h"

int childrenCount( int proc_nr )
{
	int children = 0;
	int i = 0;
	for (i=0; i< NR_PROCS; ++i)
		if ((mproc[i].mp_flags & IN_USE) && proc_nr != i && (mproc[i].mp_parent == proc_nr))
			++children;
	return children;
}

void maxChildren( int * children, pid_t * who )
{
	int maxChildren = -1;
	pid_t found = -1;
	int count = 0;
	int proc_nr = 0;
	for (proc_nr = 0; proc_nr < NR_PROCS; ++proc_nr)
	{
		if (mproc[proc_nr].mp_flags & IN_USE)
		{
			count = childrenCount( proc_nr );
			if (count > maxChildren)
			{
				maxChildren = count;
				found = mproc[proc_nr].mp_pid;
			}
		}
	}
	*children = maxChildren;
	*who = found;
}

PUBLIC int do_maxChildren()
{
	int children = -1;
	pid_t who = -1;
	maxChildren( & children, & who );
	return children;
}

PUBLIC int do_whoMaxChildren()
{
	int children = -1;
	pid_t who = -1;
	maxChildren( & children, & who);
	return who;
}

static void search_recursively(int *longest_path, int *longest_path_pid, int length, int proc_idx, pid_t root)
{
	int k = 0;
	if (length > *longest_path)
	{
		*longest_path = length;
		*longest_path_pid = root;
	}

	for (k=0; k < NR_PROCS; ++k)
	{
		if (k == proc_idx) continue;

		if (mproc[k].mp_parent == proc_idx)
			search_recursively(longest_path, longest_path_pid, length + 1, k, root);

	}

	return;
}

PUBLIC int do_test()
{
	int pid_to_skip = mm_in.m1_i1;
	int result = 0;
	int longest_path = 1;
	int longest_path_pid = 0;



	int i;
	for (i=0; i < NR_PROCS; ++i)
	{
		if (mproc[i].mp_flags & IN_USE && mproc[i].mp_pid != pid_to_skip)
			search_recursively(&longest_path, &longest_path_pid, 1, i, mproc[i].mp_pid);
	}


	result = longest_path;
	longest_path_pid <<= 10;
	result += longest_path_pid;


	return result;
}

/*=====================================================================*
 *			    do_reboot				       *
 *=====================================================================*/
PUBLIC int do_reboot()
{
  register struct mproc *rmp = mp;
  char monitor_code[32*sizeof(char *)];

  if (rmp->mp_effuid != SUPER_USER) return(EPERM);

  switch (reboot_flag) {
  case RBT_HALT:
  case RBT_REBOOT:
  case RBT_PANIC:
  case RBT_RESET:
	break;
  case RBT_MONITOR:
	if (reboot_size >= sizeof(monitor_code)) return(EINVAL);
	if (sys_copy(who, D, (phys_bytes) reboot_code,
		MM_PROC_NR, D, (phys_bytes) monitor_code,
		(phys_bytes) (reboot_size+1)) != OK) return(EFAULT);
	if (monitor_code[reboot_size] != 0) return(EINVAL);
	break;
  default:
	return(EINVAL);
  }

  /* Kill all processes except init. */
  check_sig(-1, SIGKILL);

  tell_fs(EXIT, INIT_PROC_NR, 0, 0);	/* cleanup init */
  tell_fs(EXIT, MM_PROC_NR, 0, 0);	/* cleanup for myself */

  tell_fs(SYNC,0,0,0);

  sys_abort(reboot_flag, MM_PROC_NR, monitor_code, reboot_size);
  /* NOTREACHED */
}

/*=====================================================================*
 *			    do_svrctl				       *
 *=====================================================================*/
PUBLIC int do_svrctl()
{
  int req;
  vir_bytes ptr;

  req = svrctl_req;
  ptr = (vir_bytes) svrctl_argp;

  /* Is the request for the kernel? */
  if (((req >> 8) & 0xFF) == 'S') {
	return(sys_sysctl(who, req, mp->mp_effuid == SUPER_USER, ptr));
  }

  switch(req) {
  case MMSIGNON: {
	/* A user process becomes a task.  Simulate an exit by
	 * releasing a waiting parent and disinheriting children.
	 */
	struct mproc *rmp;
	pid_t pidarg;

	if (mp->mp_effuid != SUPER_USER) return(EPERM);

	rmp = &mproc[mp->mp_parent];
	tell_fs(EXIT, who, 0, 0);

	pidarg = rmp->mp_wpid;
	if ((rmp->mp_flags & WAITING) && (pidarg == -1
		|| pidarg == mp->mp_pid || -pidarg == mp->mp_procgrp))
	{
		/* Wake up the parent. */
		rmp->reply_res2 = 0;
		setreply(mp->mp_parent, mp->mp_pid);
		rmp->mp_flags &= ~WAITING;
	}

	/* Disinherit children. */
	for (rmp = &mproc[0]; rmp < &mproc[NR_PROCS]; rmp++) {
		if (rmp->mp_flags & IN_USE && rmp->mp_parent == who) {
			rmp->mp_parent = INIT_PROC_NR;
		}
	}

	/* Become like MM and FS. */
	mp->mp_pid = mp->mp_procgrp = 0;
	mp->mp_parent = 0;
	return(OK); }

  case MMSWAPON: {
	struct mmswapon swapon;

	if (mp->mp_effuid != SUPER_USER) return(EPERM);

	if (sys_copy(who, D, (phys_bytes) ptr,
		MM_PROC_NR, D, (phys_bytes) &swapon,
		(phys_bytes) sizeof(swapon)) != OK) return(EFAULT);

	return(swap_on(swapon.file, swapon.offset, swapon.size)); }

  case MMSWAPOFF: {
	if (mp->mp_effuid != SUPER_USER) return(EPERM);

	return(swap_off()); }

  default:
	return(EINVAL);
  }
}
