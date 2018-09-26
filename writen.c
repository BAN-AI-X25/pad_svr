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
#include "writen.h"
#include <sys/errno.h>

static int Max_Error_Write = 3;

//write all nbytes to x25 sock with first qbait
//nbytes and nwritten must be >=2 !

int
writensockint(int fd,char *ptr,int nbytes,int (*inter_handler)(void),int (*error_handler)(void)) {
	int qbait;
	int nleft, nwritten;
	int cerr = 0;

	if (nbytes < 2) return(0);
	qbait = *ptr;
	nleft = nbytes - 1;
	while (nleft > 0) {
		*ptr = qbait;
		nwritten = write(fd, ptr, nleft + 1);
		if (nwritten <= 0) {
			if (errno == EINTR) {
				//Write Socket interrapted
				if (inter_handler() == 0)
					return(nbytes - nleft);
			}
			else {
				//write error
				cerr++;
				Perror("Write error to x25 sock:");
				if (cerr < Max_Error_Write) {
					Printf(1,"Write error to x25 sock: Will not exit. Left to write %i\n",nleft + 1);
					//return(nbytes - nleft);
				}
				else {
					error_handler();// do error handler
					return(nbytes - nleft);
				}
			}
		}
		else {
			cerr = 0;
			nleft -= nwritten;
			nleft++;
			ptr   += nwritten;
			ptr--;
		}
	}
	return(nbytes - nleft);
}

//write all nbytes to fd
int
writen(int fd,char *ptr,int nbytes) {
	int	nleft, nwritten;

	nleft = nbytes;
	while (nleft > 0) {
		nwritten = write(fd, ptr, nleft);
		if (nwritten <= 0)
			return -1;//INTR or error
		nleft -= nwritten;
		ptr   += nwritten;
	}
	return nbytes - nleft;
}
