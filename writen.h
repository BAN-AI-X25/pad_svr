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
#ifndef WRITEN_H
#define WRITEN_H
#include "all.h"

int
writen(int fd,char *ptr,int nbytes);

int
writensockint(int sock,char* bufd,int len,int (*inter_handler)(void),int (*error_handler)(void));


#endif
