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
#include "issue.h"
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <termios.h>

static int insc(char *buf,int size,char **where,char c);
static int insstr(char *buf,int size,char **where,char *src);

size_t strlen(const char *s);
size_t strcspn(const char *s, const char *reject);
size_t strspn(const char *s, const char *reject);


int
issue_out() {

 	struct utsname kerninfo;
 	if (uname(&kerninfo) == -1) {
 		Perror("Cant get info about system:");
 		return(-1);
 	}
// 	printf("sysname %s\n",kerninfo.sysname);
// 	printf("nodename %s\n",kerninfo.nodename);
// 	printf("release %s\n",kerninfo.release);
// 	printf("version %s\n",kerninfo.version);
// 	printf("machine %s\n",kerninfo.machine);

	char buf[1000];
	char bufout[1000];
	FILE *fp;
	char buft[100];


	if ((fp = fopen("/etc/issue.net", "r")) == NULL) {
		Perror("Cant open /etc/issue.net file:");
		return(-1);
	}

	size_t cps;
	char cc;
	char pcc;
	char *it;
	char *itout;
	size_t slen;
	int flagc = 0;
	//bzero(bufout,sizeof(bufout));
	memset(bufout,'-',sizeof(bufout));


	while (fgets(buf,sizeof(buf),fp) != NULL) {
		//printf("Buf|%s|\n",buf);
		it = buf;
		itout = bufout;

		while ( (cc = *it) != 0) {
			//printf("flag: %i c '%c'\n",flagc,cc);
			if (flagc == 1) {
				if (cc != pcc) {
					switch (cc) {
						case '\n':
							break;
						case 'l'://     - show the current tty
							{
								int kod = ttyname_r(STDIN_FILENO, buft, sizeof(buft));
								if (kod) {
									*buft = 0;
									Perror("Cant get ttyname:");
									//Exit(1);
								}
								insstr(bufout,sizeof(bufout),&itout,buft);
							}
							break;
						case 'h':
						case 'n'://- show the system node name (FQDN)
							insstr(bufout,sizeof(bufout),&itout,kerninfo.nodename);
							break;
						case 'D':
						case 'o'://- show the name of the NIS domain
							break;
						case 'd':
						case 't'://- show the current time and date
							{
								time_t ti;
								time_t ct = time(0);
								struct tm tim;
								localtime_r(&ct,&tim);
								strftime (buft,100,"%m/%d %H:%M:%S ",&tim);
								insstr(bufout,sizeof(bufout),&itout,buft);
							}
							break;
						case 's'://- show the name of the operating system
							insstr(bufout,sizeof(bufout),&itout,kerninfo.sysname);
							break;
						case 'm'://- show the machine (hardware) type
							insstr(bufout,sizeof(bufout),&itout,kerninfo.machine);
							break;
						case 'r'://- show the operating system release
							insstr(bufout,sizeof(bufout),&itout,kerninfo.release);
							break;
						case 'v'://- show the operating system version
							insstr(bufout,sizeof(bufout),&itout,kerninfo.version);
							break;
					}
				}
				else {
					insc(bufout,sizeof(bufout),&itout,cc);
				}
				flagc = 0;
			}
			else {

				if ( cc == '\\' ||  cc == '%') {
					flagc = 1;
				}
//				else if (cc != '\n') {
				else {
					flagc = 0;
					insc(bufout,sizeof(bufout),&itout,cc);
				}

// 				insc(bufout,sizeof(bufout),&itout,cc);
// 				cc = *it;
			}

			pcc = cc;
			it++;
		}
		*itout = 0;
//		printf("out|%s|\n",bufout);
		printf("%s",bufout);
		fflush(stdout);
	}

	fclose(fp);
}

static int
insc(char *buf,int size,char **where,char c) {
	if ((*where - buf) < size - 1) {
		**where = c;
		(*where)++;
	}
}

static int
insstr(char *buf,int size,char **where,char *src) {
	int ost,len;
	ost = size - 1 - ((*where - buf) + strlen(src));
	if (ost > 0) {
		len = strlen(src);
	}
	else {
		len = size - 1 - (*where - buf);
	}
	//printf("ost %i len %i \n",ost,len);
	memcpy(*where,src,len);
	//printf("our|%s|\n",buf);
	(*where) += len;

}











//
