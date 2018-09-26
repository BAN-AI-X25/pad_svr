# Make for x25 server

pad_svr :pad_svr3.o do_copy.o open_x25.o x25trace.o x29proto.o \
	ptypair.o sighandler.o loging.o watchdog.o leave_pid.o cfgfile.o \
	issue.o cmdopt.o writen.o monitor.o do_utmp.o pts_addr.o
	gcc -g -O -Wall -D_REENTRANT -lrt -o pad_svr pad_svr3.o do_copy.o open_x25.o x25trace.o x29proto.o \
	ptypair.o sighandler.o loging.o watchdog.o leave_pid.o cfgfile.o \
	issue.o cmdopt.o writen.o monitor.o do_utmp.o pts_addr.o
clean	:
	rm pad_svr pad_svr3.o do_copy.o open_x25.o x25trace.o x29proto.o \
	ptypair.o sighandler.o loging.o watchdog.o leave_pid.o cfgfile.o \
	issue.o cmdopt.o writen.o monitor.o do_utmp.o pts_addr.o

install :
	rm /etc/pad_svr/pad_svr
	cp pad_svr /etc/pad_svr
	/sbin/service pad_svr restart
