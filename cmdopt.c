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
#include "cmdopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

extern char *Myname;
extern char *Name_cfg_file;

void print_usage(void) {
	Printf(0,"Usage:   %s [name_cfg_file|stop|reread]\n",Myname);
	Printf(0,"  -  If cfg file no exist - it create with defaults.\n");
	Printf(0,"  -  stop - stop deamon.\n");
	Printf(0,"  -  reread - reread cfg file and close/open (new) log file,\n");
	Printf(0,"  -  -    (new x.121 address not accepting and new x.25 socket is not opening.)\n");

}

void gopt(int argc,char *argv[]) {
	if (argc == 2)  {
		//printf("argv %s\n",argv[1]);
		if (strcmp(argv[1],"stop") == 0) {
			//printf("stop\n");
			int wpid;
			wpid = exist_pid();
			//printf("wpid %i\n",wpid);
			if (wpid == 0) {
				Printf(0,"%s is not working.\n",Myname);
				Exit(1);
			}
			else {
				Printf(0,"Sending SIGTERM to group pid %i\n",wpid);
				killpg(wpid,SIGTERM);
				Exit(0);
			}
		}
		else if (strcmp(argv[1],"reread") == 0) {
			//printf("reread\n");
			int wpid;
			wpid = exist_pid();
			//printf("wpid %i\n",wpid);
			if (wpid == 0) {
				Printf(0,"%s is not working.\n",Myname);
				Exit(1);
			}
			else {
				Printf(0,"Sending SIGUSR1 to group pid %i\n",wpid);
				killpg(wpid,SIGUSR1);
				Exit(0);
			}
		}
		else if (strlen(argv[1]) < MAXFILENAME)
			strcpy(Name_cfg_file,argv[1]);
		else {
			printf("Very long name of cfg file\n");
		}
	}
	else if (argc != 1) {
		print_usage();
		Exit(1);
	}
}

