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
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "watchdog.h"

extern long Watchdog_timeout;
extern long Net_timeout;
extern int Forwardtimer;

static timer_t Wtimer;
static struct itimerspec Wits;
static struct sigevent wsevent;// Dnt touch this. Why need static? - den.zavg 
static timer_t Rtimer;
static struct itimerspec Rits;
static struct sigevent rsevent;// Dnt touch this. Why need static? - den.zavg 

void
cre_timers() {
	wsevent.sigev_notify = SIGEV_SIGNAL;
	wsevent.sigev_signo = SIGALRM;
	wsevent.sigev_value.sival_int = 1;

	if (timer_create(CLOCK_REALTIME,&wsevent,&Wtimer) != 0) {
		Perror("Cant create interval timer:");
		Exit(1);
	}
	bzero(&Wits,sizeof(Wits));

	rsevent.sigev_notify = SIGEV_SIGNAL;
	rsevent.sigev_signo = SIGRTMIN;
	rsevent.sigev_value.sival_int = 1;

	if (timer_create(CLOCK_REALTIME,&rsevent,&Rtimer) != 0) {
		Perror("Cant create interval timer:");
		Exit(1);
	}
	bzero(&Rits,sizeof(Rits));

}

void
set_watch_dog (void) {
// 	struct itimerval iti;
// 	bzero(&iti,sizeof(iti));
// 	iti.it_value.tv_sec = Watchdog_timeout;
// 	if ((setitimer(ITIMER_REAL,&iti,NULL)) != 0) {
// 		Perror("Cant set Watch Dog interval timer.");
// 		Exit(1);
// 	}

	Wits.it_value.tv_sec = Watchdog_timeout;
	if (timer_settime(Wtimer,0,&Wits,NULL) != 0) {
		Perror("Cant set Watch Dog interval timer:");
		Exit(1);
	}

}

void
set_read_timeout (void) {
	Rits.it_value.tv_nsec = Forwardtimer * 5000000;

	if (timer_settime(Rtimer,0,&Rits,NULL) != 0) {
		Perror("Cant set interval timer for Forward_idle_timeout:");
		Exit(1);
	}
}

void
reset_read_timeout (void) {
	Rits.it_value.tv_nsec = 0;

	if (timer_settime(Rtimer,0,&Rits,NULL) != 0) {
		Perror("Cant reset interval timer for Forward_idle_timeout:");
		Exit(1);
	}
}


void
set_net_timeout (void) {
// 	struct itimerval iti;
// 	bzero(&iti,sizeof(iti));
// 	iti.it_value.tv_sec = Net_timeout;
// 	if ((setitimer(ITIMER_REAL,&iti,NULL)) != 0) {
// 		Perror("Cant set interval timer.");
// 		Exit(1);
// 	}

	Wits.it_value.tv_sec = Net_timeout;
	if (timer_settime(Wtimer,0,&Wits,NULL) != 0) {
		Perror("Cant set interval timer.");
		Exit(1);
	}
}

void
reset_timer (void) {
// 	struct itimerval iti;
// 	bzero(&iti,sizeof(iti));
// 	iti.it_value.tv_sec = 0;
// 	if ((setitimer(ITIMER_REAL,&iti,NULL)) != 0) {
// 		Perror("Cant reset interval timer.");
// 		Exit(1);
// 	}

	Wits.it_value.tv_sec = 0;
	if (timer_settime(Wtimer,0,&Wits,NULL) != 0) {
		Perror("Cant reset interval timer.");
		Exit(1);
	}
}
