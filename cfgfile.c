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
#include "cfgfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int writecfg();
static int faclen(int packet_len);


static const char *litadr = "local_x121_address";
static const char *litlvl = "log_level";
static const char *litlog = "log_path";
static const char *litwch = "Watchdog_timeout";

static const char *litwin = "Winsize_in";
static const char *litpin = "Pacsize_in";
static const char *litifo = "Forward_idle_timeout";
static const char *litx29 = "x29_profile";
static const char *litbuf = "Buffer_size";

//static const char *litthr = "Throughput";


extern char *Chaddr;
extern int Log_level;
extern char *Name_log_file;
extern char *Name_cfg_file;
extern int Watchdog_timeout;

extern int Winsize_in;
extern int Pacsize_in;
extern int Throughput;
extern int Forwardtimer;

extern struct x29_par X29_profile[];
extern int X29_profile_len;

extern int Buffer_size;

void get_chaddr(char *num) {
}

int
readcfg() {
	FILE *file;
	if ( (file = fopen(Name_cfg_file,"r")) == NULL) {
		Perror("Cant open cfg file for read:");
		Printf(0,"Create default cfg file: %s\n",Name_cfg_file);
		if (!writecfg()) {
			return(0);
		}
	}
	else {
		char buf[200];
		char *ptrtok1;
		while (fgets(buf,sizeof(buf),file) != NULL) {
			//printf("cfg file %s",buf);
			if (*buf != '#' && *buf != '\n') {
				//printf("YEs\n");
				char *lit = strtok_r(buf, " \t\n",&ptrtok1);
				char *val = strtok_r(NULL, " \t\n",&ptrtok1);
				if (lit != NULL && val != NULL) {
					//Printf(0,"%s      %s\n",lit,val);
					if (strcmp(lit,litadr) == 0) {
						size_t len;
						if ((len = strlen(val)) <= 15) {
							int i = 0;
							int fla = 1;
							for (i = 0;i < len;i++) {
								//printf("%c",*(num + i));
								if (isdigit(*(val + i)) == 0) {
									Printf(0,"X.121 address consists of up to 15 digit\n");
									fla = 0;
								}
							}
							if (fla == 1) strcpy(Chaddr,val);
						}
						else {
							Printf(0,"X.121 address consists of up to 15 digit\n");
							break;
						}
	
					}
					else if (strcmp(lit,litx29) == 0) {
						//Printf(0,"x29 profile %s\n",val);
						char *ptrtok2;
						char *ptrtok3;

						char *parnum;
						char *parval;
						char *partok = strtok_r(val,",",&ptrtok2);
						int toklen;
						int cpar = 0;
						int iparnum,iparval;
						do {
							toklen = strlen(partok);
							if (toklen > 2 && toklen < 7) {
								//Printf(0,"partok %s\n",partok);
								parnum = strtok_r(partok,":",&ptrtok3);
								parval = strtok_r(NULL,":",&ptrtok3);
								if  (parnum != NULL && parval != NULL) {
									//Printf(0,"parnum:%s  parval:%s\n",parnum,parval);
									iparnum = atoi(parnum);
									iparval = atoi(parval);
									if (iparnum >= 1 && iparnum <= 30 && iparval >= 0 && iparval <= 255) {
										X29_profile[cpar].par_ref = iparnum;
										X29_profile[cpar].par_value = iparval;
										cpar++;
									}
								}
							}
						}
						while ((partok = strtok_r(NULL,",",&ptrtok2)) != NULL);
						X29_profile_len = cpar - 1;
					}
					else if (strcmp(lit,litlvl) == 0) {
						Log_level = atoi(val);
						if (Log_level < 0) Log_level = 0;
						if (Log_level > 10) Log_level = 10;
					}
					else if (strcmp(lit,litlog) == 0) {
						strcpy(Name_log_file,val);
					}
					else if (strcmp(lit,litwch) == 0) {
						Watchdog_timeout = atoi(val);
						if (Watchdog_timeout < 1) Watchdog_timeout = 1;
						if (Watchdog_timeout > 600) Watchdog_timeout = 600;
					}
					else if (strcmp(lit,litwin) == 0) {
						Winsize_in = atoi(val);
						if (Winsize_in < 1) Winsize_in = 1;
						if (Winsize_in > 7) Winsize_in = 7;
					}
					else if (strcmp(lit,litpin) == 0) {
						Pacsize_in = atoi(val);
						Pacsize_in = faclen(Pacsize_in);
						if (Pacsize_in < 4) Pacsize_in = 4;
						if (Pacsize_in > 12) Pacsize_in = 12;
					}
					else if (strcmp(lit,litifo) == 0) {
						Forwardtimer = atoi(val);
						if (Forwardtimer < 0) Forwardtimer = 0;
						if (Forwardtimer > 199) Forwardtimer = 199;
					}
					else if (strcmp(lit,litbuf) == 0) {
						Buffer_size = atoi(val);
						if (Buffer_size < 0) Buffer_size = 16000;
					}
	// 				else if (strcmp(lit,litthr) == 0) {
	// 					Throughput = atoi(val);
	// 				}
				}
			}
		}
	}
	fclose(file);
	return(1);
}

void
echocfg(void) {
	Printf(0,"Local x121 address: %s\n",Chaddr);
	Printf(0,"Log level: %i\n",Log_level);
	Printf(0,"Name of log file: %s\n",Name_log_file);
	Printf(0,"Watchdog timer: %i\n",Watchdog_timeout);
	Printf(0,"Winsize_in:  %i\n",Winsize_in);
	Printf(0,"Pacsize_in:  %i\n",paclen(Pacsize_in));
	Printf(0,"Forward_idle_timeout:  %i\n",Forwardtimer);

	x29profile_echo();
	Printf(0,"Buffer_size:  %i\n",Buffer_size);
//	Printf(0,"Throughput:  %i\n",Throughput);
}

static int
writecfg() {
	FILE *file;
	if ( (file = fopen(Name_cfg_file,"w+")) == NULL) {
		Perror("Cant open cfg file for create/write:");
		return(0);
	}

	fprintf(file,"#This is config file for pad_svr program.\n");
	fprintf(file,"#\n");
	fprintf(file,"%s %s\n",litadr,Chaddr);
	fprintf(file,"%s %i\n",litlvl,Log_level);
	fprintf(file,"%s %s\n",litlog,Name_log_file);
	fprintf(file,"%s %i\n",litwch,Watchdog_timeout);
	fprintf(file,"%s %i\n",litwin,Winsize_in);
	fprintf(file,"%s %i\n",litpin,paclen(Pacsize_in));
	fprintf(file,"%s %i\n",litifo,Forwardtimer);
	fprintf(file,"%s %s\n",litx29,"1:0,2:0,3:0,4:1,13:4,15:0");
	fprintf(file,"%s %i\n",litbuf,Buffer_size);

//	fprintf(file,"%s = %i\n",litthr,Throughput);

	fclose(file);
	return(1);
}

int
paclen(int faci) {
	return(1 << faci);
}

static int
faclen(int packet_len) {
	int i = 0;
	while ((packet_len >>= 1) != 0) {
		i++;
	}
	return(i);
}








//





















//sadddddddddd
