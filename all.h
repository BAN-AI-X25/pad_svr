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
#ifndef ALL_H
#define ALL_H
#include "pad_svr.h"
#include "open_x25.h"
#include "do_copy.h"
#include "sighandler.h"
#include "x29proto.h"
#include "loging.h"
#include "ptypair.h"
#include "x25trace.h"
#include "leave_pid.h"
#include "cfgfile.h"
#include "cmdopt.h"
#include "watchdog.h"
#include "writen.h"
#include "issue.h"
#include "monitor.h"
#include "pts_addr.h"


//Buf size for read from x25 socket and write to pts
//#define BUFSIZE 32000
//Buf size with QBIT  bait
//#define BUFSIZE_Q BUFSIZE+1

#define MAXFILENAME 200
#define MAXX29PROFILE 25
#define MAXLINE 1000

#endif
