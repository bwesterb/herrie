/*
 * Copyright (c) 2006-2007 Ed Schouten <ed@fxq.nl>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/**
 * @file audio_output_alsa.c
 * @brief ALSA audio output driver.
 */

#include "stdinc.h"

#include <alsa/asoundlib.h>

#include "audio_file.h"
#include "audio_output.h"

static snd_pcm_t		*devhnd;
static snd_pcm_hw_params_t	*devparam;

int
audio_output_open(void)
{
	int ret;
	unsigned int srate = 44100;
	GString *errstr;

	/* Open the device */
	if ((ret = snd_pcm_open(&devhnd, "default",
	    SND_PCM_STREAM_PLAYBACK, 0)) != 0)
		goto error;

	/* Retreive the hardware parameters */
	if ((ret = snd_pcm_hw_params_malloc(&devparam)) != 0)
		goto error;
	if ((ret = snd_pcm_hw_params_any(devhnd, devparam)) != 0)
		goto error;

	/* Set the access method - XXX: mmap */
	if ((ret = snd_pcm_hw_params_set_access(devhnd, devparam,
	    SND_PCM_ACCESS_RW_INTERLEAVED)) != 0)
		goto error;

	/* XXX: We want 16 bits little endian, 2 channels, 44.1 KHz */
	if (snd_pcm_hw_params_set_rate_near(devhnd, devparam, &srate,
	    NULL) != 0)
		goto error;
	if ((ret = snd_pcm_hw_params_set_format(devhnd, devparam,
	    SND_PCM_FORMAT_S16_LE)) != 0)
		goto error;
	if (snd_pcm_hw_params_set_channels(devhnd, devparam, 2) != 0)
		goto error;
	
	/* Now apply the parameters and get started */
	if (snd_pcm_hw_params(devhnd, devparam) != 0)
		goto error;
	if (snd_pcm_prepare(devhnd) != 0)
		goto error;
	
	return (0);
error:
	errstr = g_string_new("");
	g_string_printf(errstr, _("Cannot open the audio device: %s.\n"),
	    snd_strerror(ret));
	g_printerr(errstr->str);
	g_string_free(errstr, TRUE);
	return (-1);
}

int
audio_output_play(struct audio_file *fd)
{
	char buf[3760];
	size_t len;
	snd_pcm_sframes_t lenf, ret;

	if ((len = audio_file_read(fd, buf, sizeof buf)) == 0)
		return (-1);
	
	lenf = len / 4;

	if ((ret = snd_pcm_writei(devhnd, buf, lenf)) != lenf)
		return (-1);
	
	return (0);
}

void
audio_output_close(void)
{
}