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
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



int get_master_pty(char **name) {
	int master = -1;
	char *slavename;
	
	master = open("/dev/ptmx", O_RDWR);
	
	if (master >= 0 && grantpt(master) >= 0 && unlockpt(master) >= 0) {
		slavename = (char *)ptsname(master);
		if (!slavename) {
			close(master);
			Perror("Get name slave pty:");
			Exit(1);
		} else {
			*name = strdup(slavename);
			return master;
		}
	}
	Perror("Cant open pty:");
	Exit(1);
}


int get_slave_pty(char * name) {
	int slave = -1;
	
	slave = open(name, O_RDWR);
	
	return(slave);
}


