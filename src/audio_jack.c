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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <jack/jack.h>

#include "ptrim.h"
#include "audio.h"

#define JACK_CLIENT_NAME "plasmatrim"

#define ZERO_dB 7

#define RGB(rr,gg,bb) {.r = rr, .g = gg, .b = bb}

struct rgb colors[8] = {
	[0 ... ZERO_dB - 2] = RGB(0x00, 0xFF, 0x00),
	[ZERO_dB - 2 ... ZERO_dB - 1] = RGB(0xFF, 0xFF, 0x00),
	[LEDS - 1] = RGB(0xFF, 0x00, 0x00)
};

struct audio_state {
	jack_client_t *client;
	jack_port_t *inport;
	float *buffer;
};

struct audio_state state = {
	.client = NULL,
	.inport = NULL,
	.buffer = NULL
};

static void copy_rgb_with_brightness(struct rgb *dst, struct rgb *src,
		float brightness)
{
	dst->r = lrintf(src->r * brightness);
	dst->g = lrintf(src->g * brightness);
	dst->b = lrintf(src->b * brightness);
}

static void set_leds(float magnitude)
{
	int i, top;

	magnitude += (ZERO_dB * LEDS);
	magnitude /= LEDS;

	top = lrintf(floorf(magnitude));

	for (i = 0; i < top; i++)
		copy_rgb_with_brightness(&leds[i], &colors[i], 1.f);

	copy_rgb_with_brightness(&leds[i], &colors[i], magnitude - top);

	for (i++; i < LEDS; i++) {
		leds[i].r = 0x0;
		leds[i].g = 0x0;
		leds[i].b = 0x0;
	}
}

static int process(jack_nframes_t nframes, void *unused)
{
	float *in, rms, sum, dB;
	int i;

	in = jack_port_get_buffer(state.inport, nframes);

	for (sum = 0, i = 0; i < nframes; i++)
		sum += powf(in[i], 2);

	rms = sqrtf(sum / nframes);
	dB = 20 * log10f(rms);
	set_leds(dB);
}

static void shutdown(void *unused)
{
	puts("jack shutting down.");
	exit(0);
}

static void deactivate()
{
	jack_deactivate(state.client);
}

static int buffer_size(jack_nframes_t nframes, void *unused)
{
	if (!state.buffer) {
		if (!(state.buffer = calloc(nframes, sizeof(*state.buffer)))) {
			puts("calloc() failed.");
			exit(1);
		}
	} else {
		if (!realloc(state.buffer, nframes * sizeof(*state.buffer))) {
			puts("realloc() failed.");
			exit(1);
		}
	}

	return 0;
}

static int activate()
{
	int i;

	for (i = 0; i < LEDS; i++) {
		leds[i].r = 0x0;
		leds[i].g = 0x0;
		leds[i].b = 0x0;
	}

	if (jack_activate(state.client)) {
		puts("jack_activate() failed.");
		return -1;
	}

	/* XXX: temporary */
	jack_connect(state.client, "moc:output0", jack_port_name(state.inport));

	atexit(deactivate);
}

int audio_init()
{
	jack_status_t status;

	state.client = jack_client_open(
		JACK_CLIENT_NAME, JackNoStartServer, &status, NULL);

	if (!state.client) {
		puts("jack_client_open() failed.");
		return -1;
	}

	state.inport = jack_port_register(
		state.client, "analysis:input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

	jack_set_process_callback(state.client, process, NULL);
	jack_set_buffer_size_callback(state.client, buffer_size, NULL);
	jack_on_shutdown(state.client, shutdown, NULL);

	return activate();
}
