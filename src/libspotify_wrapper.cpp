/**
 * @file spotify.cpp
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

#include "spotify.h"

#define DEBUGLEVEL 0
#define DEBUG if(DEBUGLEVEL)

static sp_playlist_callbacks pl_callbacks = {
    &tracks_added,
    &tracks_removed,
    &tracks_moved,
    &playlist_renamed,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

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
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitLoggedOutSignal(session);
    DEBUG printf("Logged out of Spotify...\n");
}

extern "C" void notify_main_thread(sp_session *)
{
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
    DEBUG printf("log_message: %s\n", data);
}

extern "C" void end_of_track(sp_session * /*session*/)
{
    SpotWorker *sw = SpotWorker::getInstance();
    sw->endOfTrack();
}

extern "C" void stop_playback(sp_session * /*session*/)
{
    printf("%s:%s: not implemente!\n", __FILE__, __func__);
}

extern "C" void sigIgn(int signo)
{
    DEBUG printf("sigIgn: %d\n", signo);
}

extern "C" void metadata_updated(sp_session * /*session*/)
{
}

extern "C" void session_ready(sp_session * session)
{
    DEBUG printf("Session ready called!\n");
    SpotWorker *sw = SpotWorker::getInstance();
    sw->emitSessionReadySignal(session);
}

extern "C" void search_complete(sp_search * search, void * /*userdata*/)
{
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
    //TODO?:
    // sp_search_release(sw->g_search);
    // terminate();
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

extern "C" void container_loaded(sp_playlistcontainer *playlists,
                                 void * /*userdata*/)
{
    printf("SpotWorker: container_loaded (%d playlists)\n",
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
    DEBUG printf("%d tracks were added to a playlist\n", num_tracks);
    fflush(stdout);
}

extern "C" void tracks_removed(sp_playlist * /*pl*/, const int * /*tracks*/,
                               int num_tracks, void * /*userdata*/)
{
    DEBUG printf("%d tracks were removed from a playlist\n", num_tracks);
    fflush(stdout);
}

extern "C" void tracks_moved(sp_playlist * /*pl*/,
                             const int * /*tracks*/,
                             int num_tracks,
                             int /*new_position*/,
                             void * /*userdata*/)
{
    DEBUG printf("%d tracks were moved around in a playlist\n", num_tracks);
    fflush(stdout);
}

extern "C" void playlist_renamed(sp_playlist *pl, void * /*userdata*/)
{
    const char *name = sp_playlist_name(pl);
    DEBUG printf("Playlist was renamed to \"%s\".\n", name);
    // TODO: sw->emitPlaylistAdded(playlists);
}
