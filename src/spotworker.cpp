/**
 * @file spotworker.cpp
 * @author Pål Driveklepp <jalla2000@gmail.com>
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2009-2011
 *
 * @section license_sec License
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

#include <libgen.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QMessageBox>
#include <QIODevice>
#include <QTextStream>
#include <QMessageBox>

#include "spotworker.hpp"
#include "alsaworker.hpp"
#include "soundsaver.hpp"

#define DEBUGLEVEL 1
#define DEBUG if(DEBUGLEVEL)

bool SpotWorker::instanceFlag_ = false;
SpotWorker* SpotWorker::workerInstance_ = NULL;

static sp_session_callbacks g_callbacks = {
    &logged_in,
    &logged_out,
    &metadata_updated,
    &connection_error,
    NULL,
    &notify_main_thread,
    &music_delivery,
    &play_token_lost,
    &log_message,
    &end_of_track,
    NULL,
    NULL,
    &stop_playback,
    NULL,
    NULL,
    NULL
};

static sp_playlistcontainer_callbacks pc_callbacks = {
    &playlist_added,
    &playlist_removed,
    &playlist_moved,
    &container_loaded
};

SpotWorker* SpotWorker::getInstance()
{
    if (!instanceFlag_)
    {
        workerInstance_ = new SpotWorker();
        instanceFlag_ = true;
        return workerInstance_;
    }
    else
    {
        return workerInstance_;
    }
}

SpotWorker::SpotWorker(QObject *parent)
    : QObject(parent)
{

    totalFrames_ = 0;
    frameCounter_ = -1;
    //playing = false;

    DEBUG printf("SpotWorker started\n");
}

int SpotWorker::start(QString username, QString password)
{
    sp_session_config config;
    sp_error error;
    QByteArray ba;

    config.api_version = SPOTIFY_API_VERSION;
    ba = settings.value("spotify/cache").toString().toLocal8Bit();
    config.cache_location = strdup(ba.data());
    ba = settings.value("spotify/config").toString().toLocal8Bit();
    config.settings_location = strdup(ba.data());
    config.application_key = g_appkey;
    config.application_key_size = g_appkey_size;
    config.user_agent = "libspotify-client";
    config.compress_playlists = false;
    config.dont_save_metadata_for_playlists = false;
    config.initially_unload_playlists = false;

    config.callbacks = &g_callbacks;

    error = sp_session_create(&config, &currentSession);
    if (SP_ERROR_OK != error) {
        fprintf(stderr, "failed to create session: %s\n",
                sp_error_message(error));
    }

    // TODO: add callback for error checking to adjust to API changes
    sp_session_login(currentSession,
                     username.toUtf8().data(),
                     password.toUtf8().data());

    eventTimer = new QTimer();
    connect(eventTimer, SIGNAL(timeout()), SLOT(processEvents()));
    eventTimer->start(3000);
    watchDog_ = new QTimer();
    connect(watchDog_, SIGNAL(timeout()), SLOT(streamingStopped()));

    soundSaver_ = new SoundSaver();
    alsaWorker_ = new AlsaWorker();
    DEBUG printf("Spotworker started with soundSaver and alsaWorker constructed\n");

    return 0; //TODO: return something else when stuff fail.
}

void SpotWorker::performSearch(QString query)
{
    QByteArray qba = query.toUtf8();
    const char *needle = qba.data();
    printf("Requesting search. Query: %s\n", needle);
    const int track_offset = 0;
    const int track_count = 100;
    const int album_offset = 0;
    const int album_count = 100;
    const int artist_offset = 0;
    const int artist_count = 100;

    g_search = sp_search_create(currentSession,
                                needle,
                                track_offset,
                                track_count,
                                album_offset,
                                album_count,
                                artist_offset,
                                artist_count,
                                search_complete,
                                NULL);

    if (!g_search) {
        fprintf(stderr, "Clay Davis says: Sheeeet! Failed to start search!\n");
    }
}

void SpotWorker::loadPlayer(sp_track *track, bool rip, SoundSaver::FileType type)
{
    totalFrames_ = sp_track_duration(track)*44;
    alsaWorker_->audioFifoFlush();
    frameCounter_ = 0;

    closeFile();
    if(rip)
        saveFile(track, type);

    sp_session_player_load(currentSession, track);
}
void SpotWorker::playPlayer(bool play)
{
    sp_session_player_play(currentSession, play);
    alsaWorker_->pause(!play);
}
void SpotWorker::seekPlayer(int offset)
{
    sp_session_player_seek(currentSession, offset);
}
bool SpotWorker::isPlaying()
{
    return alsaWorker_->isPlaying();
}
bool SpotWorker::isStreaming()
{
    return this->streaming_;
}


void SpotWorker::processEvents()
{
    int timeout = 1000;
    sp_session_process_events(currentSession, &timeout);
    eventTimer->setInterval(timeout);
}

void SpotWorker::emitConnectionErrorSignal(sp_session *session, sp_error error)
{
    DEBUG printf("Emitting signal connectionError\n");
    emit connectionError(session, error);
}

void SpotWorker::emitLoggedInSignal(sp_session *session, sp_error error)
{
    if (SP_ERROR_OK != error) {
        fprintf(stderr, "SpotWorker: Login failed: %s\n",
                sp_error_message(error));
    }
    else{
        printf("SpotWorker: Successfully logged in\n");

        emit loggedIn(session, error);

        sp_playlistcontainer *playlists = sp_session_playlistcontainer(session);
        void * userdata = NULL;
        sp_playlistcontainer_add_callbacks(playlists,
                                           &pc_callbacks,
                                           userdata);

        int listCount = sp_playlistcontainer_num_playlists(playlists);
        printf("%d playlists discovered\n", sp_playlistcontainer_num_playlists(playlists));

        if(listCount > 0)
            emit playlistAdded(playlists);

        for (int i = 0; i < listCount && i < 10; ++i) {
            sp_playlist *pl = sp_playlistcontainer_playlist(playlists, i);

            //TODO: register playback callback
            //sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);

            if (sp_playlist_is_loaded(pl)){
                DEBUG printf("Playlist found: %s (%i tracks)\n",
                             sp_playlist_name(pl), sp_playlist_num_tracks(pl));
            }
        }
	DEBUG {
	    if (listCount > 10)
		printf("...and %d more.\n", listCount-10);
	}
    }
}

void SpotWorker::emitLoggedOutSignal(sp_session *session)
{
    DEBUG printf("Emitting signal loggedOut\n");
    emit loggedOut(session);
}

int SpotWorker::emitMusicDeliverySignal(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
{
    int framesEaten = alsaWorker_->musicDelivery(session, format, frames, num_frames);
    frameCounter_ += framesEaten;
    //check if the song is buffered completely

    /* Check if end of song is reached
     * If the samplerate is 44.1kHz, we should get about 44100 frames per second.
     * This is approximately 44.1 samples per millisecond. If the difference
     * totalFrames-frameCouner < threshold, the song is probably buffered.
     * In addition, a timer is used for extra safety.
     */
    //printf("Frames left: %d\n", totalFrames_ - frameCounter_);
    if(totalFrames_ - frameCounter_ > 100){
        watchDog_->start(1000);
    }

    soundSaver_->saveSound(frames, framesEaten);
    return framesEaten;
}

void SpotWorker::emitPlayTokenLostSignal(sp_session *session)
{
    emit playTokenLost(session);
}

void SpotWorker::endOfTrack()
{
    fprintf(stderr, "Reached end of track at %i of %i frames\n",
            frameCounter_, totalFrames_);
}

void SpotWorker::emitSessionReadySignal(sp_session *session)
{
    emit sessionReady(session);
}

void SpotWorker::emitSearchCompleteSignal(sp_search *search)
{
    emit searchComplete(search);
}

void SpotWorker::emitSessionTerminatedSignal(void)
{
    DEBUG printf("Emitting signal sessionTerminated\n");
    emit sessionTerminated();
}

void SpotWorker::emitPlaylistAdded(sp_playlistcontainer *playlists)
{
    emit playlistAdded(playlists);
}





/**
 * Print the given track title together with some trivial metadata
 *
 * @param  track   The track object
 */

/*
void SpotWorker::print_track(sp_track *track)
{
        int duration = sp_track_duration(track);

        printf("  Track \"%s\" [%d:%02d] has %d artist(s), %d%% popularity\n",
                sp_track_name(track),
                duration / 60000,
                (duration / 1000) / 60,
                sp_track_num_artists(track),
                sp_track_popularity(track));
}
*/


void SpotWorker::streamingStopped()
{
    watchDog_->stop();
    DEBUG printf("Streamstop signal received\n");
    soundSaver_->close();
    streaming_ = false;
}

void SpotWorker::playbackStopped()
{
    //this->playing = false;
}

void SpotWorker::saveFile(sp_track *track, SoundSaver::FileType nextFile)
{
    /* Close previous file
     * This should be done by both the soundsaver and the spotworker
     * so this code is probably triple redundant. Wear seatbelts!
     */
    if(soundSaver_){
        DEBUG printf("Closing open file...\n");
        soundSaver_->close();
        DEBUG printf("Closed!\n");
    }

    //start a new file
    DEBUG printf("Extracting filename from track...\n");
    sp_artist *tartist = sp_track_artist(track, 0);
    const char *artistName = sp_artist_name(tartist);
    const char *trackName = sp_track_name(track);
    QString fileName("./");
    fileName += QString(QString().fromUtf8(artistName)) +
        " - " + QString(QString().fromUtf8(trackName));
    DEBUG printf("Making new soundsaver...\n");
    soundSaver_->open(fileName.toUtf8().data(), nextFile);
    DEBUG printf("Soundsaver made. returning...\n");
}

void SpotWorker::closeFile()
{
    //close previous file
    if(soundSaver_){
        DEBUG printf("Closing open file...");
        soundSaver_->close();
        DEBUG printf("Closed!\n");
    }
}

 /*
   void SpotWorker::ResetCounter()
   {
   frameCounter = 0;
   }
 */

int SpotWorker::getProgress()
{
    return frameCounter_/44; //%TODO: dirty constant
}

int SpotWorker::getSongLength()
{
    return totalFrames_/44;
}


void SpotWorker::startServer()
{
    tcpServer_ = new QTcpServer(this);

    if (!tcpServer_->listen(QHostAddress::Any, 2718)) {
        /*      QMessageBox::warning(this, tr("Error"),
                              tr("Failed to start server: %1.")
                             .arg(anyWho->errorString()));
        */
        DEBUG printf("SpotWorker::startServer(): Epic error!\n");
        return;
    }
    DEBUG printf("TCP server started!\n");
    connect(tcpServer_, SIGNAL(newConnection()), this, SLOT(netConnection()) );
}

void SpotWorker::netConnection()
{
    DEBUG printf("Connection initiated!\n");
    serverData_.state = 0;
    serverData_.xfered = 0;
    clientConnection_ = tcpServer_->nextPendingConnection();
    connect(clientConnection_, SIGNAL(disconnected()),
            clientConnection_, SLOT(deleteLater()));
    connect(clientConnection_, SIGNAL(readyRead()),
            this, SLOT(rxDataReady()) );
}

void SpotWorker::rxDataReady()
{
    qint64 amount = this->clientConnection_->bytesAvailable();
    qint64 handled = 0;

    DEBUG printf("Incoming data! Amount: %lld\n", amount);

    while (handled < amount){

        if (serverData_.state==0 && amount>=4){
            char buf[4];
            qint64 received = clientConnection_->read(buf, 4);
            handled += received;
            for(int i = 0; i < 4; i++)
                ((char *)&serverData_.type)[i] = buf[3-i];
            DEBUG printf("Packet received. Type: %d\n", serverData_.type);
            serverData_.state = 1;
        }

        if (serverData_.state==1 && amount>=4){
            char buf[4];
            qint64 received = clientConnection_->read(buf, 4);
            handled += received;
            for(int i = 0; i < 4; i++)
                ((char *)&serverData_.length)[i] = buf[3-i];
            DEBUG printf("Packet size: %d\n", serverData_.length);
            if(serverData_.length > MAX_PACKET_SIZE)
                DEBUG printf("Length of packet is crazy. Client is a bastard!");
            serverData_.state = 2;
        }
        if (serverData_.state == 2){
            if(serverData_.xfered==serverData_.length){
                DEBUG printf("Complete packet received\n");
                parsePacket();
            }
            else {
                qint64 received = clientConnection_->read((char *)&serverData_.data, serverData_.length-serverData_.xfered);
                handled += received;
                //printf("Packet received. Type: %d\n", serverData.type);
                serverData_.xfered += received;
                printf("%lld bytes added to buffer\n", received);
            }
        }
    }
}

void SpotWorker::parsePacket()
{
    DEBUG {
        printf("packet type: %d\n", serverData_.type);
        printf("Playstop: %d\n", PLAYSTOP);
        printf("Nextblock: %d\n", NEXTBLOCK);
    }

    switch(serverData_.type){
    case PLAYSTOP:
        playPlayer(!isPlaying());
        break;
    case NEXTBLOCK:
        DEBUG printf("TODO: send block of audio data\n");
        break;
    default:
        DEBUG printf("Package had illegal type. Dropping.\n");
    }

    serverData_.state = 0;
    serverData_.xfered = 0;
}
