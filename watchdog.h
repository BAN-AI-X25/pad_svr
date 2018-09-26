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
#ifndef WATCHDOG_H
#define WATCHDOG_H
#include "all.h"

void set_watch_dog(void);
void reset_timer(void);
void set_net_timeout(void);
void cre_timers(void);
void set_read_timeout(void);
void reset_read_timeout(void);



#endif
