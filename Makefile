-include .config

INSTALL ?= install
LOCAL_CFLAGS += -I.
# Installation paths

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

#CFLAGS += -MMD -Wall

LDLIBS_ASOUND ?= -lasound
LDLIBS_OPUS ?= -lopus
LDLIBS_ORTP ?= -lortp -L. -lmobvoisdk -lpthread

LDLIBS += $(LDLIBS_ASOUND) $(LDLIBS_OPUS) $(LDLIBS_ORTP)

.PHONY:		all install dist clean

all:		 main

main:   rx.o device.o sched.o kws.o 


rx:		rx.o device.o sched.o

tx:		tx.o device.o sched.o

install:	rx tx
		$(INSTALL) -d $(DESTDIR)$(BINDIR)
		$(INSTALL) rx tx $(DESTDIR)$(BINDIR)

dist:
		mkdir -p dist
		V=$$(git describe) && \
		git archive --prefix="trx-$$V/" HEAD | \
			gzip > "dist/trx-$$V.tar.gz"

clean:
		rm -f *.o *.d tx rx

-include *.d
