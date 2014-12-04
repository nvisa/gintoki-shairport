CONFIG_AO=yes
CONFIG_ALSA=yes
CONFIG_HAVE_GETOPT_H=yes
CFLAGS+=     -I/usr/include/alsa  
LDFLAGS+= -lm -lpthread -lssl -lcrypto   -lao   -lasound  
