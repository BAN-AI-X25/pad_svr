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
#include <sys/socket.h>   
//#include <sys/time.h>    
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/x25.h>
#include <sys/errno.h>
#include <string.h>
#include <unistd.h>
//#include <signal.h>
#include <sys/types.h>
#include <linux/sockios.h>

//#include <sys/wait.h>

#include "open_x25.h"
#include "all.h"

extern char *Chaddr;
extern int Listen_sock;

extern int Winsize_in;
extern int Winsize_out;
extern int Pacsize_in;
extern int Pacsize_out;
extern int Throughput;

extern int DatalengthMin;
//extern int DatalengthMax;

int
open_listen(void) {
	int result,extended = 0; /*x25 subscription field */

	/* X25 data structures */
	struct sockaddr_x25 serverAddr,clientAddr;
	struct x25_facilities facilities;
	struct x25_subscrip_struct subscription;
	struct x25_calluserdata cud;
	struct x25_subaddr sub;
// 	cdte_address rdte_addr, /*remote calling address extension */
// 		ldte_addr; /* local calling address extension */

	struct x25_dte_facilities cud_dte;

	int debug_on = 1;

	/* set up addresses */
	bzero((char *)&serverAddr, sizeof(serverAddr));
//wait_signal();
	/* set up listening socket */
	Printf(5,"Creating an x25 socket\n");
	Listen_sock = socket(AF_X25, SOCK_SEQPACKET, 0);
	//Printf(0,"Listen_sock %i\n",Listen_sock);
//wait_signal();
	if(Listen_sock == -1) {
		Perror("Cant create x25 socket:");
		Exit(1);
	}

	serverAddr.sx25_family = AF_X25;
	strcpy(serverAddr.sx25_addr.x25_addr,Chaddr);

	result = bind(Listen_sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr));	
	if(result == -1) {
		Perror("Cant socket bind:");
		Exit(1);
	}
//wait_signal();
	/* set  up subscription */
	unsigned long facil_mask = (
					//X25_MASK_REVERSE |
		       			X25_MASK_THROUGHPUT |
					X25_MASK_PACKET_SIZE |
				       	X25_MASK_WINDOW_SIZE |
				       	X25_MASK_CALLING_AE |
				       	X25_MASK_CALLED_AE);

	subscription.global_facil_mask = facil_mask;
	subscription.extended = extended;
	strcpy(subscription.device, "x25tap0");

	Printf(5,"Setting up x25 subscriptions\n");
	result =  ioctl(Listen_sock, SIOCX25SSUBSCRIP, &subscription);
	if (result == -1){
		Perror("Set x25 subscription:");
		Exit(1);
	}

	/* set up facilities */
	facilities.winsize_in = Winsize_in;
	facilities.winsize_out = Winsize_in;
	facilities.pacsize_in = Pacsize_in;
	facilities.pacsize_out = Pacsize_in;

	facilities.throughput = Throughput;
	/* set reverse to 0x01 to reject reverse charging */
	//facilities.reverse=0x00;
	Printf(5,"Setting facilities\n");
	result =  ioctl(Listen_sock,SIOCX25SFACILITIES,&facilities);
	if(result == -1){
		Perror("set facilities:");
		Exit(1);
	}

	/* This is optional, set to 0 if not required */
	cud.cudlength = 4;
	/* match first two octets of call user data */
	/* This is optional, set to 0 if not required */
	sub.cudmatchlength = 4;

	if(cud.cudlength) {
		cud.cuddata[0] = 0x01;	
		cud.cuddata[1] = 0x00;
		cud.cuddata[2] = 0x00;
		cud.cuddata[3] = 0x00;

		Printf(5,"Setting call user data \n");
		result = ioctl(Listen_sock, SIOCX25SCALLUSERDATA, &cud);
		if(result == -1){
			Perror("set calluserdata:");
			Exit(1);
		}

		/* Accept only calls that match the first 
		   'cudmatchlength' bytes of call user data */
		if(sub.cudmatchlength) {
			result = ioctl(Listen_sock,SIOCX25SCUDMATCHLEN, &sub);
			if(result == -1) {
				Perror("set cud matchleng:");
				Exit(1);
			}
		}
	}

	/* This is optional set to 0 if not required */
// 	Printf(5,"Setting call accept approval on x25 socket\n");
// 	result = ioctl(Listen_sock,SIOCX25CALLACCPTAPPRV);
// 	if(result == -1) {
// 		Perror("accept approve error:");
// 		exit(1);
// 	}

	/* Listen of incomming conections */
	Printf(1,"Listineing for calls on %s\n", serverAddr.sx25_addr.x25_addr);
	result = listen(Listen_sock, 300);
	if(result == -1){
		Perror("listen error");
		Exit(1);
	}

	int yes = 1;
	setsockopt(Listen_sock,SOL_X25,X25_QBITINCL,&yes,sizeof(yes));//We will control  X25 Qbit
}

void
echo_x25(int sock) {
	int result;
	struct x25_facilities facilities;

	/* get up facilities */
	Printf(5,"Getting facilities\n");
	result =  ioctl(sock,SIOCX25GFACILITIES,&facilities);
	if(result == -1){
		Perror("get facilities:");
		Exit(1);
	}
//	DatalengthMax = paclen(facilities.pacsize_out) * facilities.winsize_out;
	DatalengthMin = paclen(facilities.pacsize_out);

	Printf(3,"X.25: winsize_in:%i, winsize_out:%i, pacsize_in:%i, pacsize_out:%i\n",facilities.winsize_in,facilities.winsize_out,paclen(facilities.pacsize_in),paclen(facilities.pacsize_out));
	Printf(10,"X.25: throughput  %i\n",facilities.throughput);

// 	facilities.winsize_in;
// 	facilities.winsize_out;
// 	facilities.pacsize_in;
// 	facilities.pacsize_out;
//	facilities.throughput;
//	facilities.reverse;



}

// void
// send_call_accept(int sock) {
// 	int result;
// 	/* Send a manual call accept */
// 	result = ioctl(sock,SIOCX25SENDCALLACCPT);
// 	if(result == -1) {
// 		Perror("send call accept:");
// 		exit(1);
// 	}
// 
// }
// 





