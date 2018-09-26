/*		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 *
 * 		This program is distributed in the hope that it will be useful,
 * 		but WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *		GNU General Public License for more details.
 */ 
#include "monitor.h"
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/poll.h>

extern char *Myname;
extern int Flag_Iamboss;
extern int Flag_do_pts_addr;
//extern int Moni_pipefd[2];
extern int Fd_fifo;
extern int Flag_Hup;
extern int Flag_Chld;
//extern int Flag_Alrm;
extern int Flag_Exit;
extern int Flag_Io;
extern int Flag_Usr1;
extern int Fd_fifo_rezult;

void
do_monitor()
{
	int	pid_main_work;
	int	stat;
	int	flag_need_start = 1;
	siginfo_t siginfo;
	int	Done = 1;
	int	flag_need_mes = 0;
	int	i;
	sigset_t ss_all,ss_no,ss_old;

	open_fifo_rd();

	sigfillset(&ss_all);
	sigfillset(&ss_no);
	sigdelset(&ss_no,SIGCHLD);
	sigdelset(&ss_no,SIGHUP);
	sigdelset(&ss_no,SIGIO);
	sigdelset(&ss_no,SIGUSR1);

	sigprocmask(SIG_BLOCK,&ss_all,&ss_old);

	while (Done) {
		if (flag_need_start == 1) {
			pid_main_work = fork();
			flag_need_start = 0;
			flag_need_mes = 1;
		}
		
		if (pid_main_work == -1) {// fork error
			Perror("Monitor: fork failed: ");
			Exit(1);
		}
		else if (pid_main_work == 0) {//We are in main_work
			Flag_Iamboss = 0;
			sigprocmask(SIG_SETMASK,&ss_old,NULL);
			do_main_work();
			Exit(0);
		}
		else {//                      Stay in Monitor
			if (flag_need_mes == 1) {
				Printf(5,"Monitor is watching to %s (%i)\n",Myname,pid_main_work);
				flag_need_mes = 0;
			}

			//Printf(0,"Mon waiting signal \n");
			sigsuspend(&ss_no);
			//Printf(0,"Mon arrived signal \n");

			if (Flag_Chld == 1) {
				//Printf(0,"Monitor:Chld\n");
				Flag_Chld = 0;
				wait(&stat);
				if (WIFEXITED(stat) != 0) {
					if (WEXITSTATUS(stat) == 0) {//Child exit normally
						Printf(0,"Monitor: %s (%i) stoped.\n",Myname,pid_main_work);
						break;
						//flag_need_start = 1;//Its for testing
					}
					else {//child return Error
						Printf(0,"Monitor: %s (%i) will restart after error.\n",Myname,pid_main_work);
						flag_need_start = 1;
					}
				}
				else {
					Printf(0,"Monitor: %s (%i) terminated.\n",Myname,pid_main_work);
					break;
				}
			}
			if (Flag_Hup == 1) {
				//Printf(0,"Monitor:Hup\n");
				Flag_Hup = 0;
				readcfg();
				closeFdlog();
				openFdlog();
				//echocfg();
			}
			if (Flag_Io == 1) {
				Flag_Io = 0;
				read_fifo();
			}
			if (Flag_Usr1 == 1) {
				Flag_Usr1 = 0;
				//Printf(0,"Monitor:SIGUSR1\n");
				open_fifo_rezult();
				upload_pts_addr_tab();
				close_fifo_rezult();
			}

		}
	}//end while()

	Printf(5,"Monitor Stop.\n");
	Exit(0);
}
