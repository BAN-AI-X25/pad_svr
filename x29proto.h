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
#ifndef X29PROTO_H
#define X29PROTO_H
#include "all.h"
#define X29InvitationToClear 1
#define X29IndicationOfBreak 3
#define X29Set 2

struct x29_par {
	unsigned char par_ref;
	unsigned char par_value;
};

int x29set(int fd,struct x29_par *pars,int len);
int x29set_default(int fd);
int x29inv_clear(int fd);
int x29ind_break(int fd);

void x29profile_echo(void);

#endif
