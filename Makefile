
arch ?= linux

CFLAGS ?= -O2 -Wall
-include config-${arch}.mk

PREFIX ?= /usr/local

SRCS := shairport.c daemon.c rtsp.c mdns.c mdns_external.c mdns_tinysvcmdns.c common.c rtp.c metadata.c player.c alac.c audio.c audio_dummy.c audio_pipe.c tinysvcmdns.c
SRCS += audio_tcp.c

DEPS := config-${arch}.mk alac.h audio.h common.h daemon.h getopt_long.h mdns.h metadata.h player.h rtp.h rtsp.h tinysvcmdns.h

ifdef CONFIG_SNDIO
SRCS += audio_sndio.c
CFLAGS += -DCONFIG_SNDIO
endif

ifdef CONFIG_AO
SRCS += audio_ao.c
CFLAGS += -DCONFIG_AO
endif

ifdef CONFIG_PULSE
SRCS += audio_pulse.c
CFLAGS += -DCONFIG_PULSE
endif

ifdef CONFIG_ALSA
SRCS += audio_alsa.c
CFLAGS += -DCONFIG_ALSA
endif

ifdef CONFIG_AVAHI
SRCS += mdns_avahi.c
CFLAGS += -DCONFIG_AVAHI
endif

ifdef CONFIG_HAVE_DNS_SD_H
SRCS += mdns_dns_sd.c
CFLAGS += -DCONFIG_HAVE_DNS_SD_H
endif

ifdef CONFIG_HAVE_GETOPT_H
CFLAGS += -DCONFIG_HAVE_GETOPT_H
endif

ifndef CONFIG_HAVE_GETOPT_H
SRCS += getopt_long.c
endif

# default target
all: shairport_${arch}

install: shairport_${arch}
	install -m 755 -d $(PREFIX)/bin
	install -m 755 shairport $(PREFIX)/bin/shairport

GITREV = $(shell git describe --always)
DIRTY := $(shell if ! git diff --quiet --exit-code; then echo -dirty; fi)
VERSION = \"$(GITREV)$(DIRTY)\"
CFLAGS += -DVERSION=${VERSION}

%_${arch}.o: %.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

OBJS := $(patsubst %.c,%_${arch}.o,${SRCS})

shairport_${arch}: $(OBJS)
	echo ${OBJS}
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -f shairport_* *.o
