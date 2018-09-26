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
#ifndef PAD_SVR_H
#define PAD_SVR_H

#include "all.h"

void Exit(int kod);
void close_sock(int fd);
void wait_signal(void);
void do_main_work(void);
int main(int argc,char *argv[]);

#endif
