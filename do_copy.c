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
#include <stdlib.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "all.h"

static int wait_net(int fd);
static void recv_exit_status(void);
static int do_inter(void);
static int wr_error(void);
static void wait_exitstatus(void);
//static void closefd_clear_pts_addr(int fd);

extern long Watchdog_timeout;
extern long Net_timeout;
extern int Flag_Hup;
extern int Flag_Chld;
extern int Flag_Alrm;
extern int Flag_Exit;
extern int Flag_Idle;
static int Done = 0;

extern int Forwardtimer;
extern int DatalengthMin;
//extern int DatalengthMax;
extern int Buffer_size;
static int Flag_Forwardtimer_ex;
static int Flag_exitstatus_recvd;
extern pid_t Pid_login;

extern int Chil_pipefd[];
extern int Flag_pts_addr_send_status;
extern int Log_level;



static void
wait_exitstatus(void) {
	if (Flag_exitstatus_recvd == 0) {
		Printf(5,"Waiting for exit code of login process(%i)...\n",Pid_login);
		char *buf = malloc(Buffer_size + 1);
		if (buf == NULL) {
			Perror("Can allocate so need memory:");
			Exit(1);
		}
		int i,r;
		struct pollfd ufds[1];
		ufds[0].fd = Chil_pipefd[0];
		ufds[0].events = POLLIN;
		Done = 0;

		set_net_timeout();
		while (Done == 0) {
			r = poll(ufds, 1, -1);
			//Printf(0,"poll end: %i\n",r);
	 		if (r < 0) {
				if (errno != EINTR) {
					Perror("poll() error:");
					Done = 11;
					break;
				}
	 		}
	
			if (ufds[0].revents & POLLIN) {//Read from pipe - signal arrived
				//Printf(0,"Signal Arrival\n");
				i = read(Chil_pipefd[0],buf,Buffer_size);
				if (Flag_Alrm == 1) {
					Printf(5,"Waiting: Timeout(%i) expired.Exiting...\n",Net_timeout);
					Done = 1;
					Flag_Alrm = 0;
				}
				if (Flag_Exit == 1) {
					Printf(5,"Waiting: Received exit signal.Exiting...\n");
					Done = 1;
					Flag_Exit = 0;
				}
				if (Flag_Chld == 1) {
					recv_exit_status();
					Flag_Chld = 0;
					Done = 1;
				}
			}
		}
		free(buf);
		reset_timer();
	}
}

static void
recv_exit_status(void) {
	pid_t	pid;
	int	stat;
	if ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		Flag_exitstatus_recvd = pid;
		clean_utmp();
		if (WIFEXITED(stat) != 0) {
			Printf(5,"Child/login %d finished. Exit status %i\n", pid,WEXITSTATUS(stat) );
		}
		else {
			Printf(5,"Child/login %d terminated\n", pid);
		}
	}
}

static int
do_inter(void) {
	int ret = 1;//Do not return after interrupt in writensockint()
	if (Flag_Hup == 1) {
		readcfg();
		closeFdlog();
		openFdlog();
		Flag_Hup = 0;
	}
	if (Flag_Alrm == 1) {
		Printf(5,"Watchdog timer (%i) expired.Exiting...\n",Watchdog_timeout);
		Done = 7;
		Flag_Alrm = 0;
		ret = 0;//return after interrupt in writensockint()
	}
	if (Flag_Exit == 1) {
		Printf(1,"Received exit signal.Exiting...\n");
		Done = 8;
		Flag_Exit = 0;
		ret = 0;//return after interrupt in writensockint()
	}
	if (Flag_Idle == 1) {
		Printf(6,"Forward timer expired.\n");
		Flag_Idle = 0;
		Flag_Forwardtimer_ex = 1;
	}
	if (Flag_Chld == 1) {
		recv_exit_status();
		Flag_Chld = 0;
	}
	return(ret);
}

static int
wr_error(void) {
	Done = 10;
	return(0);
}

static int
wait_net(int sock){
	//char buf[BUFSIZE_Q];
	//char *buf = malloc(BUFSIZE_Q);
	char *buf = malloc(Buffer_size + 1);
	if (buf == NULL) {
		Perror("Can allocate so need memory:");
		Exit(1);
	}

	int i;
	int r;
	struct pollfd ufds[2];
	ufds[0].fd = sock;
	ufds[0].events = POLLIN;
	ufds[1].fd = Chil_pipefd[0];
	ufds[1].events = POLLIN;
	Done = 0;

	set_net_timeout();
	while (Done == 0) {
		r = poll(ufds, 2, -1);
		//Printf(0,"poll end: %i\n",r);
 		if (r < 0) {
			if (errno != EINTR) {
				Perror("poll() error:");
				Done = 11;
				break;
			}
 		}

		if (ufds[1].revents & POLLIN) {//Read from pipe - signal arrived
			//Printf(0,"Signal Arrival\n");
			i = read(Chil_pipefd[0],buf,Buffer_size);
			if (Flag_Alrm == 1) {
				Printf(4,"Closing: Timeout(%i) expired.Exiting...\n",Net_timeout);
				Done = 1;
				Flag_Alrm = 0;
			}
			if (Flag_Exit == 1) {
				Printf(1,"Closing: Received exit signal.Exiting...\n");
				Done = 1;
				Flag_Exit = 0;
			}
			if (Flag_Chld == 1) {
				recv_exit_status();
				Flag_Chld = 0;
			}
		}

		if ((ufds[0].revents) & POLLERR) {
			Printf(0,"Closing: x25 socket POLLERR\n");
			Done = 1;
			break;
		}
		if ((ufds[0].revents) & POLLNVAL) {
			Printf(0,"Closing: x25 socket POLLNVAL\n");
			Done = 1;
			break;
		}
		if ((ufds[0].revents) & POLLHUP) {
			Printf(5,"Closing: x25 socket hang up\n");
			Done = 1;
			break;
		}
		if (ufds[0].revents & POLLIN) {//Read Socket
			i = read (sock, buf,Buffer_size + 1);
			if (i > 1) {
				char qbit = *buf;
				if (qbit == 0) {
					Printf(6,"Closing: Read from x25 socket:%i\n",i);
					hex_dump(8,10,buf,i);
				}
				else {
					Printf(6,"Closing: Read from x25 socket packet with Qbit = 1\n");
					hex_dump(4,4,buf,i);
					char x29type = *(buf+1);
					switch (x29type) {
					case X29Set:
						Printf(6,"Closing: Recv Set PAD message\n");
						//Done = 1;
						break;
					case X29InvitationToClear:
						Printf(6,"Closing: Recv X29 Invitation To Clear\n");
						//Done = 1;
						break;
					case X29IndicationOfBreak:
						Printf(6,"Closing: Recv X29 Indication Of Break\n");
						//Done = 1;
						break;
					}
				}
			}
			else {//Read error
				if (errno == EINTR) {//Read Socket interapted
				}
				else {
					Printf(5,"Closing: Read from net: %s\n",strerror(errno));
					Done = 1;
					break;
				}
			}
		}
	}
	free(buf);
	reset_timer();
}

int
do_copy(int fd,int sock) {
	int i,iw;
	Flag_exitstatus_recvd = 0;
	//char bufn[BUFSIZE_Q];
	//char *bufn = malloc(BUFSIZE_Q);
	char *bufn = malloc(Buffer_size + 1);//Buffer for read from network with Qbit. And for read for pipe.
	if (bufn == NULL) {
		Perror("Can allocate so need memory:");
		Exit(1);
	}
	char *bufn2 = bufn + 1;

	//char bufd[BUFSIZE_Q];
	//char *bufd = malloc(DatalengthMax + 1);//with Qbit
	//char *bufd = malloc(BUFSIZE_Q);//with Qbit test1
	char *bufd = malloc(Buffer_size + 1);//Buffer for read from pts with Qbit test1
	if (bufd == NULL) {
		Perror("Can allocate so need memory:");
		Exit(1);
	}
	char *bufd2 = bufd + 1;

	struct pollfd ufds[3];
	ufds[0].fd = sock;
	ufds[0].events = POLLIN;
	ufds[1].fd = fd;
	ufds[1].events = POLLIN;
	ufds[2].fd = Chil_pipefd[0];
	ufds[2].events = POLLIN;
	Done = 0;

	char *tbufd2 = bufd2;//next pointer to read(fd,...)
	int totali = 0;//filled in bufd2

	//int freesize = DatalengthMax;//free baits in bufd2
	//int freesize = BUFSIZE;//free baits in bufd2 test1
	int freesize = Buffer_size;//free baits in bufd2 test1

	Flag_Forwardtimer_ex = 0;
	int Flag_Minlimit_ex = 0;//escape Forwardtimer
	int minlimit = DatalengthMin;//float limit to escape Forwardtimer and transmit to x25 sock
	int Flag_Last_writesock = 0;
	int Flag_minlimit_incrd = 0;
	int lastreadmaster = 10;
	int lastwritesock = 10;

	int master_readed = 0;
	int socket_readed = 0;
	int flag_full_loging = 1;
	const int amount_readed_loging_off = 512;//For Log_level = 7

	cre_timers();

	while(Done == 0) {
		int r;
		set_watch_dog();

		r = poll(ufds, 3, -1);
		//Printf(0,"poll end: %i\n",r);
 		if (r < 0) {
			if (errno != EINTR) {
				Perror("poll() error:");
				Done = 11;
				break;
			}
			else {
			do_inter();
			}
 		}

		if (ufds[2].revents & POLLIN) {//Read from pipe - signal arrived
			//Printf(0,"Signal Arrival\n");
			i = read(Chil_pipefd[0],bufn,Buffer_size);
			do_inter();
		}

		if ((ufds[1].revents) & POLLERR) {
			Printf(0,"Master side pseudo tty: POLLERR\n");
			Done = 1;
			break;
		}
		if ((ufds[1].revents) & POLLNVAL) {
			Printf(0,"Master side pseudo tty: POLLNVAL\n");
			Done = 1;
			break;
		}
		if ((ufds[1].revents) & POLLHUP) {
			Printf(5,"Master side pseudo tty: hang up\n");
			Done = 3;
			//break;//07/23
		}
		//
		if ((ufds[0].revents) & POLLERR) {
			Printf(0,"x25 socket POLLERR\n");
			Done = 10;
			break;
		}
		if ((ufds[0].revents) & POLLNVAL) {
			Printf(0,"x25 socket POLLNVAL\n");
			Done = 10;
			break;
		}
		if ((ufds[0].revents) & POLLHUP) {
			Printf(5,"x25 socket POLLHUP\n");
			Done = 2;
			break;
		}

		if ((ufds[1].revents) & POLLIN) {//Read master
			if (Forwardtimer == 0) {
				//i = read (fd,bufd2,BUFSIZE);
				i = read (fd,bufd2,Buffer_size);
				//i = read (fd,bufd2,DatalengthMax);
			}
			else {
				i = read (fd,tbufd2,freesize);
			}

			if (i > 0) {//read normal baits
				if (Forwardtimer == 0) {//dont use Forwardtimer
					Printf(6,"Read from master side pts:%i\n",i);
					Printf(6,"Write to x.25 sock:%i\n",i + 1);
					if (flag_full_loging == 1 && Log_level == 7) {
						hex_dump(0,0,bufd,i + 1);
					}
					else {
						hex_dump(8,10,bufd,i + 1);
					}
					//hex_dump(8,10,bufd,i + 1);
					iw = writensockint(sock,bufd,i + 1,do_inter,wr_error);
				}
				else {//Forwardtimer != 0,  use Forwardtimer
					freesize -= i;
					tbufd2 = tbufd2 + i;
					totali += i;
					lastreadmaster = i;
					Printf(6,"Read from master side pts:%i. total in buf:%i \n",i,totali);
					if (Flag_Last_writesock == 1) {//test1
						//minlimit = lastwritesock + i/2;
						minlimit = lastwritesock + totali/2;
						Flag_minlimit_incrd = 1;
						Printf(6,"up minlimit: %i\n",minlimit);
					}
					if (totali >= minlimit) {
						Flag_Minlimit_ex = 1;
						reset_read_timeout();
					}
					else {
						set_read_timeout();
					}
				}
				if (flag_full_loging == 1) {
					master_readed += i;
					if (master_readed > amount_readed_loging_off) flag_full_loging = 0;
				}
			}
			else if (i == 0) {
				Printf(5,"Read EOF from master side pts.\n");
				Done = 6;
			}
			else {//Read from master error
				if (errno == EINTR) {//Read from master interapted
					do_inter();
				}
				else {
					Perror("Read error from master side pts:");
					Done = 1;
					//break;//07/18
				}
			}

		}//if Read master end

		if ((ufds[0].revents) & POLLIN) {//Read Socket
			//i = read (sock, bufn, BUFSIZE_Q);
			i = read (sock, bufn,Buffer_size + 1);
			if (i == (Buffer_size + 1)) {
				Printf(0,"The buffer of X25 data filled completely(Buffer_size). Loss of data is possible.\n");
			}
			if (i > 1) {
				char qbit = *bufn;
				if (qbit == 0) {
					Flag_Last_writesock = 0;
					Printf(6,"Read from x25 socket:%i\n",i);

					if (flag_full_loging == 1 && Log_level == 7) {
						hex_dump(0,0,bufn,i);
					}
					else {
						hex_dump(8,10,bufn,i);
					}
					//hex_dump(8,10,bufn,i);
					while ((iw = writen(fd, bufn2, i-1)) < 0) {
						//Write error or INTR
						if (errno == EINTR) {
							//Write master interrapted
							do_inter();
						}
						else {
							Perror("Write error to master side pts:");
							Done = 1;
						}
					}
				}
				else {//Qbit == 1
					Printf(6,"Read from x25 socket packet with Qbit = 1\n");
					hex_dump(8,10,bufn,i);
					char x29type = *(bufn + 1);
					switch (x29type) {
					case X29InvitationToClear:
						Printf(4,"Recv X29 Invitation To Clear\n");
						set_net_timeout();
						//Done = 4;
						break;//break case!
					case X29IndicationOfBreak:
						Printf(4,"Recv X29 Indication Of Break\n");
						set_net_timeout();
						//Done = 5;
						break;//break case!
					}
				}
				if (flag_full_loging == 1) {
					socket_readed += i;
					if (socket_readed > amount_readed_loging_off) flag_full_loging = 0;
				}

			}//   if (i > 1)
			else if (i == 0) {
				Printf(5,"Read EOF from x25 socket.\n");
				Done = 9;
			}
			else {//Read from sock error  i < 0
				if (errno == EINTR) {//Read from Socket interapted
					do_inter();
				}
				else {
					Perror("Read error from x25 sock:");
					Done = 10;
				}
			}
		}// read socket
		//Printf(0,"Flag_Forwardtimer_ex:%i Flag_Minlimit_ex:%i\n",Flag_Forwardtimer_ex,Flag_Minlimit_ex);
		if ((Done == 3 || Flag_Forwardtimer_ex == 1 || Flag_Minlimit_ex == 1) && Forwardtimer != 0) {//Forwardtimer exceeded or HUP master or lot of baits in buffer
			if (totali == 0) {
				Flag_Forwardtimer_ex = 0;
				Flag_Minlimit_ex = 0;
			}
			else {
				if (Flag_Forwardtimer_ex == 1 && Flag_minlimit_incrd == 0){
					minlimit = totali - lastreadmaster*1/4;
					Printf(6,"down minlimit: %i\n",minlimit);
				}
				Flag_Forwardtimer_ex = 0;
				Flag_Minlimit_ex = 0;
				Flag_Last_writesock = 1;
				Flag_minlimit_incrd = 0;
				*bufd = 0x00;//Q bit 0
				Printf(6,"Write to x.25 sock:%i\n",totali + 1);

				if (flag_full_loging == 1 && Log_level == 7) {
					hex_dump(0,0,bufd,totali + 1);
				}
				else {
					hex_dump(8,10,bufd,totali + 1);
				}
				//hex_dump(8,10,bufd,totali + 1);
				iw = writensockint(sock,bufd,totali + 1,do_inter,wr_error);
				lastwritesock = iw;
				//Printf(6,"Write to x.25 sock end:%i\n",iw);
				tbufd2 = bufd2;
				//freesize = BUFSIZE;
				freesize = Buffer_size;
				totali = 0;
			}
		}

	}// while()
	reset_timer();
	reset_read_timeout();
	free(bufn);
	free(bufd);

	close(fd);
	send_pts_addr(getpid(),"-","-");
	Flag_pts_addr_send_status = 2;

//	Printf(6,"Done code: %i\n",Done);
	switch (Done) {
	case 10://Error read sock
	case 2://HUP sock
	case 9://read EOF sock
		close_sock(sock);
		//closefd_clear_pts_addr(fd);
		break;
	case 1://Error read master
	case 3://HUP master
	case 6://read EOF master
		//closefd_clear_pts_addr(fd);
		x29inv_clear(sock);
		//x29ind_break(sock);
		wait_net(sock);
		close_sock(sock);
		break;
	case 4://Invitation received
		//closefd_clear_pts_addr(fd);
		//x29inv_clear(sock);
		//x29ind_break(sock);
		wait_net(sock);
		close_sock(sock);
		break;
	case 5://Indication received
		//closefd_clear_pts_addr(fd);
		wait_net(sock);
		close_sock(sock);
		break;
	case 7://timeout
		//closefd_clear_pts_addr(fd);
		x29inv_clear(sock);
		//x29ind_break(sock);
		wait_net(sock);
		close_sock(sock);
		break;
	case 8://exit signal
		//closefd_clear_pts_addr(fd);
		x29inv_clear(sock);
		//x29ind_break(sock);
		wait_net(sock);
		close_sock(sock);
		break;
	case 11://error poll
		//closefd_clear_pts_addr(fd);
		x29inv_clear(sock);
		//x29ind_break(sock);
		wait_net(sock);
		close_sock(sock);
		break;
	}
	wait_exitstatus();
}

// static void
// closefd_clear_pts_addr(int fd) {
// 	close(fd);
// 	send_pts_addr(getpid(),"-","-");
// }







//

