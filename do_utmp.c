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
#include <sys/errno.h>
#include <sys/types.h>
#include <string.h>
#include <utmp.h>

#include "utmp.h"

static char *find_id(char *lite);
static char id[4];

void
add_utmp(char *tty,char *x25addr) {
	//Printf(0,"tty %s\n",tty);
//	strncpy(id,find_id(tty),4);
	//Printf(0,"Utmp id %s\n",id);

// 	struct utmp uline, *retu;
// 	if (utmpname(_PATH_UTMP)) {
// 		Printf(0,"Cant open utmp(%s) database\n",_PATH_UTMP);
// 		Exit(1);
// 	}
// 	
// 	setutent();
// 	uline.ut_type = LOGIN_PROCESS;
// //	uline.ut_pid = pid;
// 	strncpy(uline.ut_id,id,4);
// 	strncpy(uline.ut_line,tty,UT_LINESIZE);
// 	strncpy(uline.ut_host,x25addr,UT_HOSTSIZE);
// 
// 	pututline(&uline);
// 
// 	endutent();

}

void
clean_utmp () {
	//Printf(0,"Clean utmp id %s\n",id);

// 	struct utmp uline, *retu;
// 	if (utmpname(_PATH_UTMP)) {
// 		Printf(0,"Cant open utmp(%s) database\n",_PATH_UTMP);
// 		Exit(1);
// 	}
// 
// 	setutent();
// 	uline.ut_type = USER_PROCESS;
// 	strcpy(uline.ut_id,id);
// 
// 	while ((retu = getutid(&uline)) != NULL) {
// 
// 		Printf(1,"Clean in utmp pid %i\n",retu->ut_pid);
// 		retu->ut_type = DEAD_PROCESS;
// 		memset(retu->ut_user,0,UT_NAMESIZE);
// 		memset(retu->ut_host,0,UT_HOSTSIZE);
// 		retu->ut_tv.tv_sec = 0;
// 		retu->ut_tv.tv_usec = 0;
// 
// 		retu->ut_time = 0;
// 
// 		retu = pututline(retu);
// 		if (retu == NULL) {
// 			printf(0,"pututline error\n");
// 		}
// 
// 	}
// // 	else {
// // 		Printf(0,"utmp line not found\n");
// // 	}
// 
// 	endutent();


}

static char *
find_id(char *lite) {
	while (isdigit(*lite) == 0) {
		lite++;
	}
	return(lite);
}



