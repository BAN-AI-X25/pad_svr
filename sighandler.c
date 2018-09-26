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
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "sighandler.h"
#include "all.h"

static void boss_sig_chld(int signo);
static void boss_sig_exit(int signo);
static void boss_sig_hup(int signo);

static void pipe_sig_chld(int signo);
static void pipe_sig_exit(int signo);
static void pipe_sig_hup(int signo);
static void pipe_sig_alrm(int signo);
static void pipe_sig_idle(int signo);

volatile int Flag_Hup = 0;
volatile int Flag_Chld = 0;
volatile int Flag_Alrm = 0;
volatile int Flag_Exit = 0;
volatile int Flag_Idle = 0;
volatile int Flag_Io = 0;
volatile int Flag_Usr1 = 0;
int Chil_pipefd[2];
//int Moni_pipefd[2];

static sigset_t Norsigset;
extern int Flag_do_pts_addr;

//Boss inter handlers
static void
boss_sig_exit(int signo) {
	//Printf(3,"Sig_exit\n");
	Flag_Exit = 1;
}

static void
boss_sig_chld(int signo) {
	Flag_Chld = 1;
	//Printf(1,"Sig_chld\n");
}

static void
boss_sig_hup(int signo) {
	Flag_Hup = 1;
}

void
boss_setsignal(void) {
	struct	sigaction	act;
	bzero(&act,sizeof(act));

	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;//SA_RESTART;

	act.sa_handler = boss_sig_chld;
	sigaction(SIGCHLD,&act,NULL);

	act.sa_handler = boss_sig_hup;
	sigaction(SIGHUP,&act,NULL);

	act.sa_handler = boss_sig_exit;
	sigaction(SIGINT,&act,NULL);
	sigaction(SIGQUIT,&act,NULL);  /* quit (ASCII FS) */
	sigaction(SIGABRT,&act,NULL);  /* used by abort, replace SIGIOT in the future */
	sigaction(SIGTERM,&act,NULL);  /* software termination signal from kill */

	sigemptyset(&Norsigset);
}

//Child pad_svr interrupt handlers
static void
chil_sig_exit(int signo) {
	//Printf(3,"Sig_exit\n");
	Flag_Exit = 1;
	write(Chil_pipefd[1]," ",1);//All other int are blocked!
}

static void
chil_sig_chld(int signo) {
	Flag_Chld = 1;
	write(Chil_pipefd[1]," ",1);//All other int are blocked!
}

static void
chil_sig_hup(int signo) {
	Flag_Hup = 1;
	write(Chil_pipefd[1]," ",1);//All other int are blocked!
}

static void
chil_sig_alrm(int signo) {
	Flag_Alrm = 1;
	write(Chil_pipefd[1]," ",1);//All other int are blocked!
}

static void
chil_sig_idle(int signo) {
	Flag_Idle = 1;
	write(Chil_pipefd[1]," ",1);//All other int are blocked!
}

void
chil_setsignal(void) {
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss,SIGCHLD);
	sigaddset(&ss,SIGALRM);
	sigaddset(&ss,SIGHUP);
	sigaddset(&ss,SIGRTMIN);
	sigaddset(&ss,SIGINT);
	sigaddset(&ss,SIGQUIT);
	sigaddset(&ss,SIGABRT);
	sigaddset(&ss,SIGTERM);

	struct	sigaction	act;
	bzero(&act,sizeof(act));
	act.sa_mask = ss;
	act.sa_flags = 0;//SA_RESTART;

	act.sa_handler = chil_sig_chld;
	sigaction(SIGCHLD,&act,NULL);

	act.sa_handler = chil_sig_alrm;
	sigaction(SIGALRM,&act,NULL);

	act.sa_handler = chil_sig_hup;
	sigaction(SIGHUP,&act,NULL);

	act.sa_handler = chil_sig_idle;
	sigaction(SIGRTMIN,&act,NULL);

	act.sa_handler = chil_sig_exit;
	sigaction(SIGINT,&act,NULL);
	sigaction(SIGQUIT,&act,NULL);  /* quit (ASCII FS) */
	sigaction(SIGABRT,&act,NULL);  /* used by abort, replace SIGIOT in the future */
	sigaction(SIGTERM,&act,NULL);  /* software termination signal from kill */

	sigemptyset(&Norsigset);

	if (pipe(Chil_pipefd) == -1) {
		Perror("Cant create pipe:");
		Exit(1);
	}

}
//-MONITOR  pad_svr interrupt setting
static void
moni_sig_hup(int signo) {
	//Printf(0,"Mon:Sig_hup\n");
	Flag_Hup = 1;
//	write(Moni_pipefd[1]," ",1);//All other int are blocked!
}

static void
moni_sig_chld(int signo) {
	//Printf(0,"Mon:Sig_chld\n");
	Flag_Chld = 1;
//	write(Moni_pipefd[1]," ",1);//All other int are blocked!
}

static void
moni_sig_io(int signo) {
	//Printf(0,"Mon:Sig_io\n");
	Flag_Io = 1;
//	write(Moni_pipefd[1]," ",1);//All other int are blocked!
}

static void
moni_sig_usr1(int signo) {
	//Printf(0,"Mon:Sig_Usr1\n");
	Flag_Usr1 = 1;
}

void
moni_setsignal(void) {
	sigset_t ss;
	sigemptyset(&ss);
// 	sigaddset(&ss,SIGCHLD);
// 	sigaddset(&ss,SIGHUP);

	struct	sigaction	act;
	bzero(&act,sizeof(act));
	act.sa_mask = ss;
	act.sa_flags = 0;//SA_RESTART;

	act.sa_handler = moni_sig_chld;
	sigaction(SIGCHLD,&act,NULL);

	act.sa_handler = moni_sig_hup;
	sigaction(SIGHUP,&act,NULL);

	act.sa_handler = moni_sig_io;
	sigaction(SIGIO,&act,NULL);

	act.sa_handler = moni_sig_usr1;
	sigaction(SIGUSR1,&act,NULL);

// 	if (pipe(Moni_pipefd) == -1) {
// 		Perror("Cant create pipe:");
// 		Flag_do_pts_addr = 0;
// 	}

	sigemptyset(&Norsigset);

}
//---------------------------------------
void
sig_block(void) {
	sigset_t allsigset;
	sigfillset(&allsigset);
	sigprocmask(SIG_BLOCK,&allsigset,&Norsigset);
}

void
sig_unblock(void) {
	sigprocmask(SIG_SETMASK,&Norsigset,NULL);
}












//
