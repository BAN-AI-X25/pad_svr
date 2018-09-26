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
#include "pts_addr.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

extern int Flag_do_pts_addr;
extern char Name_fifo[];
int Fd_fifo = -1;
char Buf_fifo[MAXFIFOMES];
struct pts_addr_entry *Tab_first =  NULL;
struct pts_addr_entry *Tab_last = NULL;
const char Name_fifo_rezult[] = "/tmp/pts-addr.fifo";
int Fd_fifo_rezult = -1;

static struct pts_addr_entry *alloc_new();
static int ins_entry(char *pid,char *pts,char *addr);
static int echo_tab();
static struct pts_addr_entry *find_free();
static int clean_entry(char *pid);

void
setup_pts_addr(void){
	if (Flag_do_pts_addr == 1) {
		//Printf(0,"PIPE_BUF %i\n",PIPE_BUF);
		snprintf(Name_fifo,MAXFILENAME,"/tmp/pad_svr_%i.fifo",getpid());
		//snprintf(Name_fifo,MAXFILENAME,"/tmp/pad_svr.fifo");
	
		Name_fifo[MAXFILENAME - 1] = 0;
		unlink(Name_fifo);
		if (mkfifo(Name_fifo,S_IRUSR|S_IWUSR) < 0) {
			Printf(0,"Cant make fifo file %s\n",Name_fifo);
			Flag_do_pts_addr = 0;
		}
	
		unlink(Name_fifo_rezult);
		if (mkfifo(Name_fifo_rezult,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0) {
			Printf(0,"Cant make fifo file for out table pts-addr%s\n",Name_fifo_rezult);
			Flag_do_pts_addr = 0;
		}

	}
}

void open_fifo_wr(void) {
	if (Flag_do_pts_addr == 1) {
		Fd_fifo = open(Name_fifo,O_WRONLY | O_NONBLOCK);
		if (Fd_fifo == -1 && errno != ENXIO) {
			Printf(0,"Cant open fifo file %s for write\n",Name_fifo);
			Flag_do_pts_addr = 0;
		}
	}
}

void open_fifo_rd(void) {
	if (Flag_do_pts_addr == 1) {
		//Printf(0,"open fifo for rd\n");
//		Fd_fifo = open(Name_fifo,O_RDONLY);
//		Fd_fifo = open(Name_fifo,O_RDWR | O_NONBLOCK);//Open for write too: for never get hang up after close Fifo by child process.
		Fd_fifo = open(Name_fifo,O_RDONLY | O_NONBLOCK);
//		if (Fd_fifo == -1 && errno != ENXIO) {
		if (Fd_fifo == -1) {
			Printf(0,"Cant open fifo file %s for read\n",Name_fifo);
			Flag_do_pts_addr = 0;
		}
		else {
			int flags;
			if (Flag_do_pts_addr == 1 && (flags = fcntl(Fd_fifo, F_GETFL, 0)) < 0) {
				Perror("Cant get mode fifo:");
				Flag_do_pts_addr = 0;
			}
			flags |= O_ASYNC;
			if (Flag_do_pts_addr == 1 && fcntl(Fd_fifo, F_SETFL, flags) < 0) {
				Perror("Cant set ASYNC mode fifo:");
				Flag_do_pts_addr = 0;
			}
			if (Flag_do_pts_addr == 1 && fcntl(Fd_fifo,F_SETOWN,getpid()) != 0) {
				Perror("Cant set own fifo:");
				Flag_do_pts_addr = 0;
			}
				//Printf(0,"Getown fifo:%i\n",fcntl(Fd_fifo_write,F_GETOWN));


			Tab_first = malloc(sizeof(struct pts_addr_entry));
			Tab_last = Tab_first;
		
			Tab_first->pid = malloc(MAXCHARPID + 1);
			Tab_first->pts = malloc(MAXCHARPTS + 1);
			Tab_first->addr = malloc(MAXCHARADR + 1);
			Tab_first->next = NULL;
			*(Tab_first->pid) = 0;
		}
	}
}

int
send_pts_addr(pid_t pid,char *pts,char *addr) {
	if (Flag_do_pts_addr == 1) {
		int iw;
		if (addr == NULL || strlen(addr) == 0) {
			addr = "unknown";
		}
		snprintf(Buf_fifo,MAXFIFOMES,"%i %s %s",pid,pts,addr);
		//Printf(0,"Write to fifo [%s]\n",Buf_fifo);
	
		if ((iw = writen(Fd_fifo,Buf_fifo,MAXFIFOMES)) < 0) {
			//Write error or INTR
			if (errno == EINTR) {
				//Write master interrapted
			}
			else {
				Perror("Write error to fifo:");
			}
		}
		//Printf(0,"Write to fifo %i\n",iw);
	}
}

int
read_fifo() {
	int ir;
	char *ptrtok1;
	int i = 0;

	if (Fd_fifo != -1) {
		while ((ir = read(Fd_fifo,Buf_fifo,MAXFIFOMES)) > 0) {
			//if (i > 0) Printf(0,"One signal - many reads %i\n",i + 1);
			//Printf(0,"Read from fifo: %i\n",ir);
			Buf_fifo[ir] = 0;
			//Printf(0,"Read from  fifo: %s|\n",Buf_fifo);
	
			char *pid = strtok_r(Buf_fifo," \n",&ptrtok1);
			char *pts = strtok_r(NULL," \n",&ptrtok1);
			char *addr = strtok_r(NULL," \n",&ptrtok1);
			if (pid != NULL && pts != NULL && addr != NULL) {
				if (*pts == '-' && *addr == '-') {
					//Printf(0,"PID %s  delete\n",pid,pts,addr);
					clean_entry(pid);
				}
				else {
					//Printf(0,"PID %s   pts %s   addr %s\n",pid,pts,addr);
					ins_entry(pid,pts,addr);
				}
			}
			i++;
		}
	}
}

static struct pts_addr_entry *
find_free() {
	struct pts_addr_entry *ite = Tab_first;
	while(ite != NULL) {
		if (*(ite->pid) == 0) return ite;
		ite = ite->next;
	}
	return NULL;
}

static struct pts_addr_entry *
alloc_new() {
	struct pts_addr_entry *ite_new;
	ite_new = malloc(sizeof(struct pts_addr_entry));
	Tab_last->next = ite_new;
	Tab_last = ite_new;

	ite_new->pid = malloc(MAXCHARPID + 1);
	ite_new->pts = malloc(MAXCHARPTS + 1);
	ite_new->addr = malloc(MAXCHARADR + 1);
	ite_new->next = NULL;
	return ite_new;
}

static int
ins_entry(char *pid,char *pts,char *addr) {
	struct pts_addr_entry *pfree = find_free();

	if (pfree == NULL) {
		//Printf(0,"no found free, alloc\n");
		pfree = alloc_new();
	}

	strncpy(pfree->pid,pid,MAXCHARPID);
	*(pfree->pid + MAXCHARPID) = 0;
	strncpy(pfree->pts,pts,MAXCHARPTS);
	*(pfree->pts + MAXCHARPTS) = 0;
	strncpy(pfree->addr,addr,MAXCHARADR);
	*(pfree->addr + MAXCHARADR) = 0;

	//echo_tab();
}

static int
clean_entry(char *pid) {
	struct pts_addr_entry *ite = Tab_first;
	while(ite != NULL) {
		if (strcmp(ite->pid,pid) == 0) {
			*(ite->pid) = 0;
		}
		ite = ite->next;
	}

	//echo_tab();
}

static
int echo_tab() {
	struct pts_addr_entry *ite = Tab_first;
	int i = 0;
	Printf(0,"echo tab\n");
	while(ite != NULL) {
		if (*(ite->pid) != 0) {
			Printf(0,"%i %s %s %s\n",i,ite->pid,ite->pts,ite->addr);
		}
		else {
			Printf(0,"%i empty\n",i);
		}
		ite = ite->next;
		i++;
	}
	Printf(0,"echo tab end\n");
}

void
open_fifo_rezult(void) {
	if (Flag_do_pts_addr == 1) {
		Fd_fifo_rezult = open(Name_fifo_rezult,O_WRONLY | O_NONBLOCK);
		//Printf(0,"open fifo file %s for write %i\n",Name_fifo_rezult,Fd_fifo_rezult);
		if (Fd_fifo_rezult == -1) {// && errno != ENXIO
			if (errno == ENXIO) {
				Printf(0,"Cant open fifo file %s for write.Far end did'n open for read.\n",Name_fifo_rezult);
			}
			else {
				Printf(0,"Cant open fifo file %s for write\n",Name_fifo_rezult);
			}
			//Flag_do_pts_addr = 0;
		}
	}
}

void
upload_pts_addr_tab(void) {
	if (Fd_fifo_rezult != -1) {
		int iw;
		//echo_tab();
	
		struct pts_addr_entry *ite = Tab_first;
		int i = 0;
		char buf[MAXLINE];
		snprintf(buf,MAXLINE,"Num\tPID\tPts\tX.25_addr\n");
		iw = writen(Fd_fifo_rezult,buf,strlen(buf));

		while(ite != NULL) {
			if (*(ite->pid) != 0) {
				snprintf(buf,MAXLINE,"%i\t%s\t%s\t%s\n",i,ite->pid,ite->pts,ite->addr);
				buf[MAXLINE - 1] = 0;
				iw = writen(Fd_fifo_rezult,buf,strlen(buf));
				//Printf(0,"%i %s %s %s\n",i,ite->pid,ite->pts,ite->addr);
			}
			else {
				//snprintf(buf,MAXLINE,"%i\t-\t-\t-\n",i);
				//buf[MAXLINE - 1] = 0;
				//iw = writen(Fd_fifo_rezult,buf,strlen(buf));
				//Printf(0,"%i empty\n",i);
			}
			//Printf(0,"iw = %i\n",iw);
			ite = ite->next;
			i++;
		}
		snprintf(buf,MAXLINE,"End of table.\n");
		iw = writen(Fd_fifo_rezult,buf,strlen(buf));
		//Printf(0,"out fifo rezult %i\n",i);
	}
}

void
close_fifo_rezult(void) {
	if (Fd_fifo_rezult != -1) {
		close(Fd_fifo_rezult);
	}
}
























//
