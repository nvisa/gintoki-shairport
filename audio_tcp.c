/*
 * dummy output driver. This file is part of Shairport.
 * Copyright (c) James Laird 2013
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "common.h"
#include "audio.h"

static int fd;
static char *arg_host;
static int arg_port;

static int init(int argc, char **argv) {
	if (argc != 2)
		die("pipe need 2 arguments");

	arg_host = argv[0];
	sscanf(argv[1], "%d", &arg_port);

	if (arg_port == 0)
		die("port invalid");
	
	return 0;
}

static void deinit(void) {
}

static void do_connect() {
	fprintf(stderr, "connect %s:%d\n", arg_host, arg_port);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		die("socket() failed");
	}

	struct sockaddr_in si = {};
	si.sin_family = AF_INET;
	inet_aton(arg_host, &si.sin_addr);
	si.sin_port = htons(arg_port);
	socklen_t silen = sizeof(si);

	if (connect(fd, (struct sockaddr *)&si, silen) == -1) {
		die("connect %s:%d failed", arg_host, arg_port);
	}

	/*
	int v = 1024*2;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char *)&v, sizeof(v))) {
		die("setsockopt SO_SNDBUF failed");
	}
	*/
}

static void do_close() {
	fprintf(stderr, "diconnected\n");
	shutdown(fd, SHUT_RDWR);
	close(fd);
}

static long long starttime, samples_played;

static void start(int sample_rate) {
	starttime = 0;
	samples_played = 0;
	do_connect();
}

static void play(short buf[], int samples) {
	fprintf(stderr, "samples=%d\n", samples);

	int r = send(fd, buf, samples*4, 0);
	fprintf(stderr, "r=%d\n", r);
	if (r != samples*4) {
		do_close();
		return;
	}

	struct timeval tv;
	gettimeofday(&tv, NULL);
	long long nowtime = tv.tv_usec + 1e6*tv.tv_sec;
	if (!starttime)
		starttime = nowtime;
	samples_played += samples;

	long long finishtime = starttime + samples_played * 1e6 / 44100.0;
	long long sleeptime = finishtime - nowtime - (long long)(1e6 * 1);

	fprintf(stderr, "sleep %.lf ms\n", sleeptime/1e3);
	if (sleeptime > 0)
		usleep(sleeptime);
}

static void stop(void) {
	do_close();
}

static void help(void) {
    printf("    pipe takes 2 argument: [host] and [port] stream to\n");
}

audio_output audio_tcp = {
	.name = "tcp",
	.help = &help,
	.init = &init,
	.deinit = &deinit,
	.start = &start,
	.stop = &stop,
	.play = &play,
	.volume = NULL
};
