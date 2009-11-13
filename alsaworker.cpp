/*
 * (C) Copyright 2009 Pål Driveklepp
 *
 * Written by: Pål Driveklepp <jalla2000@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <alsa/asoundlib.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "queue.h"
#include "alsaworker.hpp"
#include <sndfile.h>

AlsaWorker::AlsaWorker(QObject *parent)
    : QThread(parent)
{
    g_remove_tracks = 0;
    this->audioInit();

    notifyMutex = new QMutex();
    this->paused_ = false;

}


snd_pcm_t *AlsaWorker::alsaOpen(char *dev, int rate, int channels)
{

	snd_pcm_hw_params_t *hwp;
	snd_pcm_sw_params_t *swp;
	snd_pcm_t *h;
	int r;
	int dir;
	snd_pcm_uframes_t period_size_min;
	snd_pcm_uframes_t period_size_max;
	snd_pcm_uframes_t buffer_size_min;
	snd_pcm_uframes_t buffer_size_max;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;

	printf("Bitrate: %d rate, Channels: %d\n", rate, channels);

	if ((r = snd_pcm_open(&h, dev, SND_PCM_STREAM_PLAYBACK, 0) < 0))
		return NULL;

	hwp = reinterpret_cast<snd_pcm_hw_params_t *>(alloca(snd_pcm_hw_params_sizeof()));
	memset(hwp, 0, snd_pcm_hw_params_sizeof());
	snd_pcm_hw_params_any(h, hwp);

	snd_pcm_hw_params_set_access(h, hwp, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(h, hwp, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_rate(h, hwp, rate, 0);
	snd_pcm_hw_params_set_channels(h, hwp, channels);

	/* Configurue period */

	dir = 0;
	snd_pcm_hw_params_get_period_size_min(hwp, &period_size_min, &dir);
	dir = 0;
	snd_pcm_hw_params_get_period_size_max(hwp, &period_size_max, &dir);

	period_size = 1024;

	dir = 0;
	r = snd_pcm_hw_params_set_period_size_near(h, hwp, &period_size, &dir);

	if (r < 0) {
		fprintf(stderr, "Audio: Failed to set period size %lu (%s)\n",
		        period_size, snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	dir = 0;
	r = snd_pcm_hw_params_get_period_size(hwp, &period_size, &dir);

	if (r < 0) {
		fprintf(stderr, "Audio: Unable to get period size (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	/* Configurue buffer size */

	snd_pcm_hw_params_get_buffer_size_min(hwp, &buffer_size_min);
	snd_pcm_hw_params_get_buffer_size_max(hwp, &buffer_size_max);
	buffer_size = period_size * 4;

	dir = 0;
	r = snd_pcm_hw_params_set_buffer_size_near(h, hwp, &buffer_size);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to set buffer size %lu (%s)\n",
		        buffer_size, snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	r = snd_pcm_hw_params_get_buffer_size(hwp, &buffer_size);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to get buffer size (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	/* write the hw params */
	r = snd_pcm_hw_params(h, hwp);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to configure hardware parameters (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	/*
	 * Software parameters
	 */

	swp = reinterpret_cast<snd_pcm_sw_params_t *>(alloca(snd_pcm_sw_params_sizeof()));
	memset(hwp, 0, snd_pcm_sw_params_sizeof());
	snd_pcm_sw_params_current(h, swp);

	r = snd_pcm_sw_params_set_avail_min(h, swp, period_size);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to configure wakeup threshold (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	snd_pcm_sw_params_set_start_threshold(h, swp, 0);

	if (r < 0) {
		fprintf(stderr, "audio: Unable to configure start threshold (%s)\n",
		        snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	r = snd_pcm_sw_params(h, swp);

	if (r < 0) {
		fprintf(stderr, "audio: Cannot set soft parameters (%s)\n",
		snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	r = snd_pcm_prepare(h);
	if (r < 0) {
		fprintf(stderr, "audio: Cannot prepare audio for playback (%s)\n",
		snd_strerror(r));
		snd_pcm_close(h);
		return NULL;
	}

	return h;
}

void AlsaWorker::run()
{
    printf("Starting alsaThread!\n");

    audio_fifo_t *af = &output_audiofifo;
    snd_pcm_t *h = NULL;
    int c;
    int cur_channels = 0;
    int cur_rate = 0;

    audio_fifo_data_t *afd;

    for (;;) {

	if(paused_){
	    msleep(100);
	    this->playing_ = false;
	}
	else{

	    af->mutex.lock();

	    while (!(afd = TAILQ_FIRST(&af->q)))
		af->cond.wait(&af->mutex);

	    TAILQ_REMOVE(&af->q, afd, link);
	    af->qlen -= afd->nsamples;

	    af->mutex.unlock();

	    if (!h || cur_rate != afd->rate || cur_channels != afd->channels) {
		if (h) snd_pcm_close(h);

		cur_rate = afd->rate;
		cur_channels = afd->channels;

		h = this->alsaOpen((char *)"default", cur_rate, cur_channels);

		if (!h) {
		    fprintf(stderr, "Failed to open ALSA device (%d channels, %d Hz), dying\n",
			    cur_channels, cur_rate);
		    exit(1);
		}
	    }

	    c = snd_pcm_wait(h, 1000);

	    if (c >= 0)
		c = snd_pcm_avail_update(h);

	    if (c == -EPIPE)
		snd_pcm_prepare(h);

	    snd_pcm_writei(h, afd->samples, afd->nsamples);
	    free(afd);

	    watchDog->start(1000);
	    this->playing_ = true;
	}
    }
}

void AlsaWorker::audioInit()
{
    TAILQ_INIT(&output_audiofifo.q);
    output_audiofifo.qlen = 0;

    watchDog = new QTimer();
    connect(watchDog, SIGNAL(timeout()), SLOT(playbackStopped()) );
    watchDog->start(1000);
    this->start();
}

void AlsaWorker::audioFifoFlush()
{
	audio_fifo_data_t *afd;
	audio_fifo_t *af = &output_audiofifo; //TODO:
	af->mutex.lock();

	while((afd = TAILQ_FIRST(&af->q))) {
		TAILQ_REMOVE(&af->q, afd, link);
		free(afd);
	}

	af->qlen = 0;
	af->mutex.unlock();
}

void AlsaWorker::pause(bool p)
{
    audio_fifo_t *af = &output_audiofifo;
    af->mutex.lock();
    this->paused_ = p;
    this->playing_ = !p;
    af->mutex.unlock();
}

int AlsaWorker::musicDelivery(sp_session * /*session*/,
			       const sp_audioformat *format,
			       const void *frames,
			       int num_frames)
{
    audio_fifo_t *af = &output_audiofifo;
    audio_fifo_data_t *afd;
    size_t s;

    if (num_frames == 0) {
	notifyMutex->lock();
	g_playback_done = 1;
	af->cond.wakeAll();
	notifyMutex->unlock();
	return 0;
    }

    af->mutex.lock();

    // buffer some amount of audio data
    if (af->qlen > format->sample_rate*100) {

	af->mutex.unlock();
	return 0;
    }

    s = num_frames * sizeof(int16_t) * format->channels;

    afd = reinterpret_cast<audio_fifo_data_t *>(malloc(sizeof(audio_fifo_data_t) + s));
    memcpy(afd->samples, frames, s);

    afd->nsamples = num_frames;

    afd->rate = format->sample_rate;
    afd->channels = format->channels;

    TAILQ_INSERT_TAIL(&af->q, afd, link);
    af->qlen += num_frames;

    af->cond.wakeAll();
    af->mutex.unlock();
    //printf("Ate %d samples! Buffer size: %d\n", num_frames, af->qlen);

    return num_frames;
}

bool AlsaWorker::isPlaying()
{
    return this->playing_;
}

void AlsaWorker::playbackStopped()
{
    this->playing_ = false;
}
