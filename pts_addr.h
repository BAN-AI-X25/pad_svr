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
#ifndef PTS_ADDR_H
#define PTS_ADDR_H
#include "all.h"
#include <sys/types.h>
#include <unistd.h>

#define MAXCHARPID 5
#define MAXCHARPTS 15
#define MAXCHARADR 15
//= MAXCHARPID + MAXCHARPTS + MAXCHARADR
#define MAXFIFOMES 40

struct pts_addr_entry {
	char *pid;
	char *pts;
	char *addr;
	struct pts_addr_entry *next;
//	struct pts_addr_entry *prev;
};

void setup_pts_addr(void);
void open_fifo_wr(void);
void open_fifo_rd(void);
int send_pts_addr(pid_t pid,char *pts,char *addr);
int read_fifo();
void open_fifo_rezult(void);
void upload_pts_addr_tab(void);
void close_fifo_rezult(void);

#endif
