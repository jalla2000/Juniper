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

#ifndef ALSAWORKER_H
#define ALSAWORKER_H

#include <stdint.h>
#include "queue.h"
#include <alsa/asoundlib.h>

#include <spotify/api.h>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QWaitCondition>
#include "soundsaver.hpp"

class AlsaWorker : public QThread {

    Q_OBJECT

 public:

    typedef struct audio_fifo_data {
	TAILQ_ENTRY(audio_fifo_data) link;
	int channels;
	int rate;
	int nsamples;
	int16_t samples[0];
    } audio_fifo_data_t;

    typedef struct audio_fifo {
	TAILQ_HEAD(, audio_fifo_data) q;
	int qlen;
	QMutex mutex;
        QWaitCondition cond;
    } audio_fifo_t;

    audio_fifo_t output_audiofifo;
    // Synchronization variable telling the main thread to process events
    int g_notify_do;
    // Non-zero when a track has ended and the jukebox has not yet started a new one
    int g_playback_done;
    // The global session handle
    sp_session *g_sess;
    // Handle to the playlist currently being played
    sp_playlist *g_jukeboxlist;
    // Name of the playlist currently being played
    const char *g_listname;
    // Remove tracks flag
    int g_remove_tracks;
    // Handle to the curren track
    sp_track *g_currenttrack;
    // Index to the next track
    int g_track_index;

    QMutex *notifyMutex;
    QTimer *watchDog;

    AlsaWorker(QObject *parent = 0);
    snd_pcm_t *alsaOpen(char *dev, int rate, int channels);
    void* alsaAudioStart(void *aux);
    void audioInit();
    void audioFifoFlush();
    int musicDelivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
    void pause(bool p);
    bool isPlaying();

 private:
    int frameCounter_;
    bool paused_;
    bool playing_;

 signals:
    void soundData(const void *frames, int count);

 public slots:
    void playbackStopped(void);

 protected:
    void run();

};

#endif /* ALSAWORKER_H */
