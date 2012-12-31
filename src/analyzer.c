/**
 * Copyright (c) 2012 William Light <wrl@illest.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include "ptrim.h"
#include "audio.h"

#define FPS 60

struct ptrim_color_cmd cmd = {0};
struct rgb *leds = cmd.led_colors;

static void frame_delay()
{
	usleep(lrint(FPS / 1000000.0));
}

static void throwaway_read(int dev)
{
	uint8_t buf[32];
	read(dev, buf, sizeof(buf));
}

static int write_color_cmd(int dev, struct ptrim_color_cmd *cmd)
{
	write(dev, cmd, sizeof(*cmd));
	throwaway_read(dev);
}

static int display_loop(int fd)
{
	int l, i;
	uint8_t b;

	cmd.report = 0;
	cmd.brightness = 64;

	for (l = 0;; l = (l + 1) & 0x7) {
		write_color_cmd(fd, &cmd);
		frame_delay();
	}
}

static void exit_on_sigint(int s)
{
	exit(0);
}

int main(int argc, char **argv)
{
	int dev;

	assert(argc > 1);

	if ((dev = open(argv[1], O_RDWR)) < 0) {
		perror("open() failed");
		return EXIT_FAILURE;
	}

	/* so that atexit() functions run on ctrl-c */
	signal(SIGINT, exit_on_sigint);

	if (audio_init(dev))
		return EXIT_FAILURE;

	display_loop(dev);
}
