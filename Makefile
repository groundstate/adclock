USER=root
USERGROUP=root
BINDIR = /usr/bin
CC = g++ -O2 -Wall   
RM = rm -f
DEFINES=-DDEBUG
LIBS=  

OBJS=ADClock.o Main.o GPIO.o TLV5625.o\
	tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o

all:  mkversion adclock $(OBJS) 

.cpp.o:
	$(CC) $(DEFINES) -c  $*.cpp

mkversion:
	./mkversion.pl

adclock: $(OBJS)
	$(CC) -o adclock $(OBJS) $(LIBS)

ADCLock.o: ADClock.h Debug.h Version.h tinyxml.h TLV5625.h

GPIO.o: GPIO.h

TLV5625.o: GPIO.h TLV5625.h

Main.o:	ADClock.h 

tinystr.o: tinystr.cpp
tinyxml.o:tinyxml.cpp
tinyxmlerror.o:tinyxmlerror.cpp
tinyxmlparser.o:tinyxmlparser.cpp

install:
	-/bin/cp ./adclock $(BINDIR)
	chown root.$(USERGROUP) $(BINDIR)/adclock
	mkdir -p /usr/share/adclock
	-/bin/cp adclock.xml  /usr/share/adclock
	-/bin/cp adclock.service /lib/systemd/system/
	systemctl --system daemon-reload
	systemctl enable adclock
clean:
	$(RM) *.o adclock
