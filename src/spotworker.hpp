/**
 * @file spotworker.hpp
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

#ifndef SPOTWORKER_H
#define SPOTWORKER_H

#include <QObject>
#include <QTimer>
#include <QtNetwork>
#include <stdio.h>

#include "alsaworker.hpp"
#include "spotify.h"

class QTcpServer;

#define MAX_PACKET_SIZE 10000

class SpotWorker : public QObject {

    Q_OBJECT

 public:

    sp_session *currentSession;

    QTimer *eventTimer;
    /// A global reference to the search we are currently investigating
    sp_search *g_search;
    enum PacketType { PLAYSTOP = 1,NEXTBLOCK = 2, SKIP = 3, SEEK = 4, SEARCH = 5 };

    static SpotWorker* getInstance();
    ~SpotWorker()
    {
        instanceFlag_ = false;
    }
    bool start(QString username, QString password);

    void saveUser(QString user, QString pass);

    /*
    void connection_error(sp_session *session, sp_error error);
    void logged_in(sp_session *session, sp_error error);
    void logged_out(sp_session *session);
    void notify_main_thread(sp_session *session);
    void log_message(sp_session *session, const char *data);
    void loop(sp_session *session);
    void sigIgn(int signo);
    */

    void print_track(sp_track *track);
    void print_album(sp_album *album);
    void print_artist(sp_artist *artist);
    void print_search(sp_search *search);

    /* signal-emitting C++ functions for the 9 sp_session_callbacks */
    void emitLoggedInSignal(sp_session *session, sp_error error);
    void emitLoggedOutSignal(sp_session *session);
    void emitMetadataUpdated(sp_session * session);
    void emitConnectionErrorSignal(sp_session *session, sp_error error);
    //TODO: message to user signal
    //TODO: notify main thread? (probably no need)
    int emitMusicDeliverySignal(sp_session *session,
                                const sp_audioformat *format,
                                const void *frames, int num_frames);
    void emitPlayTokenLostSignal(sp_session *session);
    //TODO: log_message signal

    /* signal emitting C++ functions for the 4 sp_playlistcontainer_callbacks */
    void emitPlaylistAdded(sp_playlistcontainer * playlists);
    //void emitPlaylistRemoved(sp_playlistcontainer * playlists);
    //void emitPlaylistMoved(sp_playlistcontainer * playlists);
    //void emitContainerLoaded(sp_playlistcontainer * playlists);

    //void emitPlaylistsDiscovered();
    void emitSessionTerminatedSignal(void);
    void emitSessionReadySignal(sp_session *session);
    void emitSearchCompleteSignal(sp_search *search);

    //These following three funnctions had sp_session *session parameter.
    //removal experimental
    void loadPlayer(sp_track *track, bool rip, SoundSaver::FileType type);
    void playPlayer(bool play);
    void resetCounter();

    void saveFile(sp_track *track, SoundSaver::FileType nextFile);
    void closeFile(void);
    int getProgress();
    int getSongLength();
    bool isPlaying();
    bool isStreaming();
    void endOfTrack();

 private:

    /* variables */
    static bool instanceFlag_;
    static SpotWorker *workerInstance_;
    SpotWorker(QObject *parent = 0);
    int totalFrames_;
    int frameCounter_;
    QTimer *watchDog_;
    bool streaming_;
    AlsaWorker *alsaWorker_;
    SoundSaver *soundSaver_;

    QTcpServer *tcpServer_;
    QTcpSocket *clientConnection_;
    typedef struct spacket {
        uint32_t state;
        uint32_t type;
        uint32_t length;
        uint32_t xfered;
        uint8_t data[MAX_PACKET_SIZE];
    } spotPacket;
    spotPacket serverData_;
    QMutex *controlMutex_;
    QSettings settings;

    /* functions */
    void parsePacket();

 public slots:
    void processEvents();
    void streamingStopped();
    void playbackStopped();
    void startServer();
    void netConnection();
    void rxDataReady();
    void seekPlayer(int offset);
    void performSearch(QString query);

 signals:
    void loggedIn(sp_session *session, sp_error error);
    void loggedOut(sp_session *session);
    void metadataUpdated(sp_session *);
    void connectionError(sp_session *session, sp_error error);
    //TODO: message to user signal
    //TODO: notify main thread? (probably no need)
    void playlistAdded(sp_playlistcontainer *plc);
    void updatePlaylistList();
    void playTokenLost(sp_session *session);
    //TODO: log_message signal

    void sessionReady(bool ready);
    void searchComplete(sp_search *search);
    void sessionReady(sp_session *session);
    void sessionTerminated(void);
};

#endif
