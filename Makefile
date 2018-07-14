#HEADER	:= sws.h extern.h
#CODE	:= checks.c main.c tools.c receiver.c send_response.c sendfunction.c

#OPTIONS	:= -Wall -Werror -g -D_GNU_SOURCE -D__USE_XOPEN

all:main.c sws.h
	gcc -Wall -Werror -g -D_GNU_SOURCE -D__USE_XOPEN -pthread -o sws checks.c main.c tools.c receiver.c sendfunction.c

#
#all: main.c send_response.o sendfunction.o checks.o tools.o sws.h extern.h
#	cc -g   main.c send_response.o sendfunction.o checks.o tools.o 

#send_response.o: send_response.c sws.h extern.h
#	cc -g -c send_response.c
#tools.o: tools.c sws.h extern.h
#	cc -g -D_GNU_SOURCE -D__USE_XOPEN -c tools.c
#receiver.o: receiver.c sws.h extern.h
#	cc -g -c receiver.c
#sendfunction.o: sendfunction.c sws.h extern.h
#	cc -g -c sendfunction.c

#checks.o: checks.c sws.h extern.h
#	cc -g -c checks.c
#clean: *.o
#	rm *.o