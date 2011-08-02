/**
 * @file spotify.c
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2011
 */

#include "spotify.h"


extern "C" void connection_error(sp_session *session, sp_error error)
{
    fprintf(stderr, "connection to Spotify failed: %s\n", sp_error_message(error));
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitConnectionErrorSignal(session, error);
}

extern "C" void logged_in(sp_session *session, sp_error error)
{
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitLoggedInSignal(session, error);

    if (SP_ERROR_OK == error) {
        // Let us print the nice message...
        sp_user *me = sp_session_user(session);
        const char *my_name = (sp_user_is_loaded(me) ?
                               sp_user_display_name(me) :
                               sp_user_canonical_name(me));
        DEBUG printf("Logged in to Spotify as user %s\n", my_name);
        session_ready(session);
    }
}

extern "C" void logged_out(sp_session *session)
{
    //if (g_exit_code< 0)
    //  g_exit_code = 0;
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitLoggedOutSignal(session);
    DEBUG printf("Logged out of Spotify...\n");
}

extern "C" void notify_main_thread(sp_session *)
{
    //pthread_kill(g_main_thread, SIGIO);
    //printf("notify_main_thred called!\n");
}

extern "C" int music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
{
    SpotWorker *sw = SpotWorker::getInstance();
    return sw->emitMusicDeliverySignal(session, format, frames, num_frames);
}

extern "C" void play_token_lost(sp_session *session)
{
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitPlayTokenLostSignal(session);
    DEBUG printf("Emitting signal playTokenLost\n");
}

extern "C" void log_message(sp_session * /*session*/, const char *data)
{
    fprintf(stderr, "log_message: %s\n", data);
}

extern "C" void end_of_track(sp_session * /*session*/)
{
    SpotWorker *sw = SpotWorker::getInstance();
    sw->endOfTrack();
}

extern "C" void stop_playback(sp_session * /*session*/){
  printf("Asked to stop playback\n");
}

extern "C" void sigIgn(int signo)
{
    DEBUG printf("sigIgn: %d\n", signo);
}

extern "C" void metadata_updated(sp_session *session)
{
    //dummy to avoid compile warning
    sp_session *jalla = session;
    jalla = jalla;
}

extern "C" void session_ready(sp_session *session)
{
    DEBUG printf("Session ready called!\n");

    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitSessionReadySignal(session);
}

extern "C" void search_complete(sp_search *search, void *userdata)
{
    //dummy to avoid compile warning
    void *jalla = userdata;
    jalla = jalla;

    SpotWorker *sw = SpotWorker::getInstance();

    //TODO: move these checks to signal receiver
    //TODO: why is *userdata not passed to signal?
    if (search && SP_ERROR_OK == sp_search_error(search)){
        sw->emitSearchCompleteSignal(search);
    }
    else{
        fprintf(stderr, "Failed to search: %s\n", sp_error_message(sp_search_error(search)));
        sw->emitSearchCompleteSignal(NULL);
    }
    //sp_search_release(sw->g_search);

    //terminate();
}

extern "C" void session_terminated(void)
{
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitSessionTerminatedSignal();
}

extern "C" void playlist_added(sp_playlistcontainer *playlists,
                               sp_playlist *addedPlaylist,
                               int position,
                               void * /*userdata*/)
{
    DEBUG printf("SpotWorker: playlist_added: (position: %d, name:%s)\n",
                 position,
                 sp_playlist_name(addedPlaylist));
    sp_playlist_add_callbacks(addedPlaylist, &pl_callbacks, NULL);

    qDebug() << "SpotWorker: new playlist" << sp_playlist_name(addedPlaylist);
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitPlaylistAdded(playlists);
}

extern "C" void playlist_removed(sp_playlistcontainer * /*pc*/,
                                 sp_playlist *pl,
                                 int position,
                                 void * /*userdata*/)
{
    printf("SpotWorker: playlist_removed (name: %s, position:%d)\n",
           sp_playlist_name(pl),
           position);
    sp_playlist_remove_callbacks(pl, &pl_callbacks, NULL);
}

extern "C" void playlist_moved(sp_playlistcontainer * /*pc*/,
                               sp_playlist *playlist,
                               int position,
                               int new_position,
                               void * /*userdata*/)
{
    DEBUG printf("SpotWorker: playlist_moved (name: %s, pos:before/after=(%d/%d))\n",
                 sp_playlist_name(playlist),
                 position,
                 new_position);
}

extern "C" void container_loaded(sp_playlistcontainer *playlists, void * /*userdata*/)
{
    DEBUG printf("SpotWorker: container_loaded (%d playlists)\n",
                 sp_playlistcontainer_num_playlists(playlists));
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitPlaylistAdded(playlists);
}


extern "C" void tracks_added(sp_playlist * /*pl*/,
                             sp_track *const * /*tracks*/,
                             int num_tracks,
                             int /*position*/,
                             void * /*userdata*/)
{
    /*
        if (pl != g_jukeboxlist)
                return;
    */
    DEBUG printf("%d tracks were added to a playlist\n", num_tracks);
    fflush(stdout);
}

extern "C" void tracks_removed(sp_playlist * /*pl*/, const int * /*tracks*/,
                               int num_tracks, void * /*userdata*/)
{
    //int i, k = 0;

    /*
    if (pl != g_jukeboxlist)
        return;

    for (i = 0; i < num_tracks; ++i)
        if (tracks[i] < g_track_index)
            ++k;

    g_track_index -= k;
    */
    DEBUG printf("%d tracks were removed from a playlist\n", num_tracks);
    fflush(stdout);
}

extern "C" void tracks_moved(sp_playlist * /*pl*/,
                             const int * /*tracks*/,
                             int num_tracks,
                             int /*new_position*/,
                             void * /*userdata*/)
{
    //TODO: Use more function parameters
    /*
        if (pl != g_jukeboxlist)
                return;
    */
        DEBUG printf("%d tracks were moved around in a playlist\n", num_tracks);
        fflush(stdout);
}

extern "C" void playlist_renamed(sp_playlist *pl, void * /*userdata*/)
{
        const char *name = sp_playlist_name(pl);

        DEBUG printf("Current playlist renamed to \"%s\".\n", name);
        //SpotWorker *sw = SpotWorker::getInstance();
        //sw->emitPlaylistAdded(playlists);

        //sp_session_player_unload(g_sess);
}
