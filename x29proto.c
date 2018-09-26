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
#include "x29proto.h"
#include "all.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static int wr_error(void);
static int do_inter(void);

extern long Watchdog_timeout;

extern int Flag_Hup;
extern int Flag_Alrm;
extern int Flag_Exit;

extern struct x29_par X29_profile[];
extern int X29_profile_len;
extern struct x29_par X29_profile_default[];
extern int X29_profile_default_len;


static int
do_inter(void) {
	if (Flag_Hup == 1) {
		readcfg();
		closeFdlog();
		openFdlog();
		Flag_Hup = 0;
	}
	if (Flag_Alrm == 1) {
		Printf(5,"Watchdog timer (%i) expired.Exiting...\n",Watchdog_timeout);
		Flag_Alrm = 0;
		Exit(0);
	}
	if (Flag_Exit == 1) {
		Printf(1,"Received exit signal.Exiting...\n");
		Flag_Exit = 0;
		Exit(0);
	}
	return(0);//return after interrupt in writensockint()
}

static int
wr_error(void) {
	return(0);
}

int
x29set(int fd,struct x29_par *pars,int len) {
}

int
x29set_default(int fd) {

	char *buf = calloc(MAXX29PROFILE,2);
	if (buf == NULL) {
		Perror("Unable alloc memory:");
		Exit(0);
	}

	int c = 0;
	*(buf + c) = 0x01;//Q bit = 1
	c++;
	*(buf + c) = X29Set;//x29 SET PAD message
	c++;

	int i;
	if (X29_profile_len == 0) {
		for (i = 0;i <= X29_profile_default_len;i++) {
			*(buf + c) = X29_profile_default[i].par_ref;
			c++;
			*(buf + c) = X29_profile_default[i].par_value;
			c++;
		}
		Printf(4,"Send x29 Set message with default x3 profile:\n");
	}
	else {
		for (i = 0;i <= X29_profile_len;i++) {
			*(buf + c) = X29_profile[i].par_ref;
			c++;
			*(buf + c) = X29_profile[i].par_value;
			c++;
		}
		Printf(4,"Send x29 Set message with x3 profile:\n");
	}

	if (i > 0) {
		hex_dump(8,10,buf,c);
		writensockint(fd,buf,c,do_inter,wr_error);
	}
	free(buf);
}

int
x29inv_clear(int fd) {
	char buf[2];

	*(buf) = 0x01;//Q bit = 1
	*(buf + 1) = X29InvitationToClear;//Inv to Clear PAD mess

	Printf(4,"Send x29 Inv to Clear:\n");
	hex_dump(8,10,buf,2);
	writensockint(fd,buf,2,do_inter,wr_error);
}

int
x29ind_break(int fd) {
	char buf[2];

	*(buf) = 0x01;//Q bit = 1
	*(buf + 1) = X29IndicationOfBreak;//Inv to Clear PAD mess

	Printf(4,"Send x29 Indi of Break:\n");
	hex_dump(8,10,buf,2);
	writensockint(fd,buf,2,do_inter,wr_error);
}

void
x29profile_echo(void) {
	int i,len;

	int size = MAXX29PROFILE * 5 + 15;
	char *buf = malloc(size);
	*buf = 0;
	if (buf == NULL) {
		Perror("Unable alloc memory:");
		Exit(0);
	}

	if (X29_profile_len > 0) {
		for (i = 0;i <= X29_profile_len;i++) {
			len = strlen(buf);
			snprintf(buf + len,size - len,"%i:%i,",X29_profile[i].par_ref,X29_profile[i].par_value);
		}
		len = strlen(buf);
		*(buf + len - 1) = 0;
		Printf(0,"x29_profile: %s\n",buf);
	}
	free(buf);
}

