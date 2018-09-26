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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <pty.h>
#include <sys/socket.h>
#include <linux/x25.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/sockios.h>
#include <pthread.h>
#include <sys/wait.h>

#include "all.h"

static void set_defaults(void);
static void init_env();
static int do_login(char *name);
static void get_myname(char *nm);
static void go_daemon();
static int do_inter (void);

char	*envinit[3];
extern char **environ;
int Listen_sock = -1;
int Accept_sock = -1;
int master = -1;

long Watchdog_timeout = 10;
long Net_timeout = 3;
int Log_level = 3;
char *Chaddr;
char *Name_log_file;
char *Name_cfg_file;
int Winsize_in = 2;//X.25
int Pacsize_in = 7;//X.25
int Throughput = 0xCC;//X.25
int Forwardtimer = 1;//X.3 parametr 4
int DatalengthMin = 128;//Real packet size X.25
int Buffer_size = 32000;
//for pts-addr table
int Flag_do_pts_addr = 1;
char Name_fifo[MAXFILENAME];
int Flag_Iamchild = 0;
int Flag_pts_addr_send_status = 0;
//end for pts-addr table



#define DEF_ADDR "35071"
#define DEF_NAME_LOG "/var/log/pad_svr.log"
#define DEF_NAME_CFG "/etc/pad_svr/pad_svr.cfg"

extern int Flag_Hup;
extern int Flag_Chld;
extern int Flag_Alrm;
extern int Flag_Exit;
extern int Flag_Idle;

char *Myname;
int Flag_Iamboss = 0;
int Flag_Logopnd = 0;
int Done = 0;

struct x29_par X29_profile[MAXX29PROFILE];
int X29_profile_len = 0;

//This profile sended by EICON card
const struct x29_par X29_profile_default[] = {
	0x01,0x00,
	0x02,0x00,
	0x03,0x00,
	0x04,0x01,//idle timer delay.
	0x0d,0x04,
	0x0f,0x00};
const int X29_profile_default_len = 6;

pid_t Pid_login;


void Exit(int kod) {
	if (Flag_Iamboss == 1) {
		clear_pid();
		if (Flag_do_pts_addr == 1 && *Name_fifo != 0) {
			unlink(Name_fifo);
		}
	}
	else if (Flag_Iamchild == 1) {
		if (Flag_pts_addr_send_status == 1) {
			send_pts_addr(getpid(),"-","-");
		}

	}
	exit(kod);
}

static void
set_defaults(void) {
	Chaddr = malloc(20);
	Name_log_file = malloc(MAXFILENAME);
	Name_cfg_file = malloc(MAXFILENAME);
	if (Chaddr == NULL || Name_cfg_file == NULL || Name_log_file == NULL) {
		Perror("Unable alloc memory:");
		Exit(1);
	}
	strcpy(Chaddr,DEF_ADDR);
	strcpy(Name_log_file,DEF_NAME_LOG);
	strcpy(Name_cfg_file,DEF_NAME_CFG);
	DatalengthMin = paclen(Pacsize_in);
//	DatalengthMax = paclen(Pacsize_in);
	*Name_fifo = 0;
}

static void
init_env() {
// 	char **envp;
// 
// 	envp = envinit;
// 	if (*envp = getenv("TZ"))
// 		*envp++ -= 3;
// 
// 	*envp = 0;

	environ = envinit;
//	putenv("TERM=network");
}

static int
do_login(char *name) {
	//struct termios ot;
	int slave;
	
	if ((slave = get_slave_pty(name)) < 0) {
		Perror("Cant open slave side pty:");
		Exit(1);
	}
	free(name);

	if (setsid() < 0) {
		Perror("Cant set new session:");
		Exit(1);
	}
	
	if (ioctl(slave, TIOCSCTTY, NULL)) {
		Perror("Cant set control tty:");
		Exit(1);
	}

// 	tcgetattr(slave, &ot);
// //	ot.c_lflag &= ~(ISIG | ECHO | ECHOCTL | ECHOE | ECHOK | ECHOKE | ECHONL | ECHOPRT);;
// //	ot.c_lflag &= ~(ICANON | ISIG | ECHO | ECHOCTL | ECHOE | ECHOK | ECHOKE | ECHONL | ECHOPRT);
// 	ot.c_iflag |= IGNBRK;
// 	ot.c_oflag |= OPOST;
// 	ot.c_oflag |= ONLCR;
// 	ot.c_oflag |= OCRNL;
// 
// 	ot.c_cc[VMIN] = 128;
// 	ot.c_cc[VTIME] = 20;
// 
// 	tcsetattr(slave, TCSANOW, &ot);

	/* сделать подчиненный pty стандартным устройством ввода, вывода и ошибок */
// 	tcflush(STDIN_FILENO,TCIOFLUSH);
// 	tcflush(STDOUT_FILENO,TCIOFLUSH);
// 	tcflush(STDERR_FILENO,TCIOFLUSH);

// 	close(STDIN_FILENO);
// 	close(STDOUT_FILENO);
// 	close(STDERR_FILENO);

	sig_block();
	dup2(slave, STDIN_FILENO);
	dup2(slave, STDOUT_FILENO);
	dup2(slave, STDERR_FILENO);

	/* в этой точке подчиненный pty должен быть стандартным устройством ввода */
	if (slave > 2) {
		close(slave);
	}
	sig_unblock();
	issue_out();

	/* запустить оболочку */
	//execl("/bin/bash", "/bin/bash",0);
	execl("/bin/login", "/bin/login","--",0);
	/* сюда управление никогда не попадет */
	Exit(1);

}

static void
get_myname(char *nm) {
	char *s;
	if ( (s = strrchr(nm,'/')) != NULL) {
		nm = s + 1;
	}
	else {
	}
	Myname = malloc(strlen(nm));
	if (Myname == NULL) {
		Perror("Unable alloc memory:");
		Exit(1);
	}
	strcpy(Myname,nm);
}

static void
go_daemon() {
        pid_t pid, sid;

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
                Exit(1);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
                Exit(0);
        }
        umask(0);

        /* Open any logs here */
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                Perror("Create new session and group:");
                Exit(1);
        }
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
		Perror("Change current dir:");
                Exit(1);
        }
        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
	int nullfd = 99;
	nullfd = open("/dev/null", O_RDWR);
	if (nullfd < 0) {
		Perror("Cant open /dev/null (but it very needed) :");
		Exit(1);
	}
	dup2(nullfd, STDIN_FILENO);
	dup2(nullfd, STDOUT_FILENO);
	dup2(nullfd, STDERR_FILENO);

	//printf("go daemon %i.",nullfd);
}

static int
do_inter (void) {
	if (Flag_Chld == 1) {
		pid_t	pid;
		int	stat;
		while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
			if (WIFEXITED(stat) != 0) {
				Printf(5,"Child %d finished. Exit status %i\n", pid,WEXITSTATUS(stat) );
			}
			else {
				Printf(5,"Child %d terminated\n", pid);
			}
		}
		Flag_Chld = 0;
	}
	if (Flag_Hup == 1) {
		readcfg();
		closeFdlog();
		openFdlog();
		echocfg();
		Flag_Hup = 0;
	}
	if (Flag_Alrm == 1) {
		//do nothing
		Flag_Alrm = 0;
	}
	if (Flag_Exit == 1) {
		Printf(1,"Received exit signal.Exiting...\n");
		Done = 1;
		Flag_Exit = 0;
	}
}

void
close_sock(int fd){
//	while(close(fd) == -1 && errno == EINTR) {
	int kod;
//	Printf(0,"Close sock %i\n",fd);
	while((kod = close(fd)) == -1) {
		if  (errno != EINTR) {
			break;
		}
	}
	//Printf(0,"Close sock kod %i\n",kod);

}

void
wait_signal(void) {//for experement only
	sigset_t smask;
	sigemptyset(&smask);
	sigsuspend(&smask);
}

void
do_main_work(void) {
	struct sockaddr_x25 clientAddr;
	pid_t pidsrv;
	char *name;

	//Printf(0,"Listen_sock %i\n",Listen_sock);
//wait_signal();
	boss_setsignal();
	open_listen();

	Done = 0;
	while(!Done) {
		Printf(5,"Accepting for calls\n");
		bzero((char *)&clientAddr, sizeof(clientAddr));
		socklen_t client_addr_len = sizeof clientAddr;
		Accept_sock = -1;

		Accept_sock = accept(Listen_sock, (struct sockaddr *)&clientAddr, &client_addr_len);
		//Printf(0,"Accept_sock %i\n",Accept_sock);
//wait_signal();
		if(Accept_sock == -1) {
			if (errno == EINTR) {
				//signal arrived
				do_inter();
				continue;
			}
			else {
				Perror("accept call:");
				Exit(1);
			}
		}
		//send_call_accept(Accept_sock);
		if ( (pidsrv = fork()) < 0) {
				Perror("fork ");
				Exit(1);
			}
//wait_signal();
		if (pidsrv == 0) {
			//we are is child svr
			sig_block();
			Flag_Iamchild = 1;
			close_sock(Listen_sock);

			//echocfg();
			if ((master = get_master_pty(&name)) < 0) {
				Perror("Cant open master side pty:");
				Exit(1);
			}
			open_fifo_wr();
			Flag_pts_addr_send_status = 1;
			send_pts_addr(getpid(),name,clientAddr.sx25_addr.x25_addr);

			Printf(2,"Accepted call from address: %s. Pts: %s\n", clientAddr.sx25_addr.x25_addr,name);
			echo_x25(Accept_sock);
			add_utmp(name,clientAddr.sx25_addr.x25_addr);
			sig_unblock();
			x29set_default(Accept_sock);


			if ((Pid_login = fork()) < 0) {
				Perror("Cant fork:");
				Exit(1);
			}
			if (Pid_login == 0) {
				//We are in child/login
				Flag_Iamchild = 0;
				init_env();
				close(master);
				do_login(name);
			}
		
			chil_setsignal();
			do_copy(master,Accept_sock);
			Printf(4,"Finished.\n");
			Exit(0);
		}//chld srv end
//------------------
		close_sock(Accept_sock);
	}//while(!Done) end
	close_sock(Listen_sock);
}

int
main(int argc,char *argv[]) {

	Flag_Iamboss = 0;
	Flag_Logopnd = 0;
	get_myname(argv[0]);

	set_defaults();
	gopt(argc,argv);

	if (exist_pid() != 0) {
		printf("Process %s already running.\n",Myname);
		Exit(1);
	}


	printf("Name of cfg file: %s\n",Name_cfg_file);

	if (!readcfg()) {
		print_usage();
		Exit(1);
	}
	openFdlog();
	echocfg();
	Printf(1,"Server %s starting...\n",Myname);
	go_daemon();

	Flag_Iamboss = 1;

	leave_pid();
	setup_pts_addr();
	moni_setsignal();

	do_monitor();

	Exit(0);
 }














//
