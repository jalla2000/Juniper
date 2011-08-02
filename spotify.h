/**
 * @file spotify.h
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2011
 */

#ifndef _SPOTIFY_H
#define _SPOTIFY_H

// The one and only entrypoint to the libspotify API
#include <libspotify/api.h>

#include "spotworker.hpp"

#define DEBUGLEVEL 1
#define DEBUG if(DEBUGLEVEL)

/* --- Data --- */
/// The application key is specific to each project, and allows Spotify
/// to produce statistics on how our service is used.
extern const uint8_t g_appkey[];
/// The size of the application key.
extern const size_t g_appkey_size;

/**
 * Callback called when libspotify has new metadata available
 *
 * Not used in this example (but available to be able to reuse the session.c file
 * for other examples.)
 */
extern "C" void metadata_updated(sp_session *session);

/**
 * Callback called when the session has successfully logged in
 *
 * This is where we start two browse requests; one artist and one album. They
 * will eventually call the album_complete() and artist_complete() callbacks.
 *
 * This
 */
extern "C" void session_ready(sp_session *session);

/**
 * Callback called when the session has been terminated.
 */
extern "C" void session_terminated(void);
extern "C" int music_delivery(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
extern "C" void play_token_lost(sp_session *session);

/**
 * This callback is called when the user was logged in, but the connection to
 * Spotify was dropped for some reason.
 *
 * @sa sp_session_callbacks#connection_error
 */
extern "C" void connection_error(sp_session *session, sp_error error);

/**
 * This callback is called when an attempt to login has succeeded or failed.
 *
 * @sa sp_session_callbacks#logged_in
 */
extern "C" void logged_in(sp_session *session, sp_error error);


/**
 * This callback is called when the session has logged out of Spotify.
 *
 * @sa sp_session_callbacks#logged_out
 */
extern "C" void logged_out(sp_session *session);

/**
 * This callback is called from an internal libspotify thread to ask us to
 * reiterate the main loop.
 *
 * The most straight forward way to do this is using Unix signals. We use
 * SIGIO. signal(7) in Linux says "I/O now possible" which sounds reasonable.
 *
 * @sa sp_session_callbacks#notify_main_thread
 */
extern "C" void notify_main_thread(sp_session *session);

/**
 * This callback is called for log messages.
 *
 * @sa sp_session_callbacks#log_message
 */
extern "C" void log_message(sp_session *session, const char *data);
extern "C" void end_of_track(sp_session *session);
extern "C" void stop_playback(sp_session *session);
extern "C" void loop(sp_session *session);

/**
 * A dummy function to ignore SIGIO.
 */
extern "C" void sigIgn(int signo);


/**
 * Callback for libspotify
 *
 * @param browse    The browse result object that is now done
 * @param userdata  The opaque pointer given to sp_artistbrowse_create()
 */
extern "C" void search_complete(sp_search *search, void *userdata);

/**
 * Callback from libspotify, telling us a playlist was added to the playlist container.
 *
 * We add our playlist callbacks to the newly added playlist.
 *
 * @param  pc            The playlist container handle
 * @param  pl            The playlist handle
 * @param  position      Index of the added playlist
 * @param  userdata      The opaque pointer
 */
extern "C" void playlist_added(sp_playlistcontainer *playlists,
                               sp_playlist *addedPlaylist,
                               int position,
                               void * userdata);

/**
 * Callback from libspotify, telling us a playlist was removed from the playlist container.
 *
 * This is the place to remove our playlist callbacks.
 *
 * @param  pc            The playlist container handle
 * @param  pl            The playlist handle
 * @param  position      Index of the removed playlist
 * @param  userdata      The opaque pointer
 */
extern "C" void playlist_removed(sp_playlistcontainer * pc,
                                 sp_playlist *pl,
                                 int position,
                                 void * userdata);

extern "C" void playlist_moved(sp_playlistcontainer * pc,
                               sp_playlist *playlist,
                               int position,
                               int new_position,
                               void * userdata);

/**
 * Callback from libspotify. Something renamed the playlist.
 *
 * @param  pl            The playlist handle
 * @param  userdata      The opaque pointer
 */
extern "C" void playlist_renamed(sp_playlist *pl, void * userdata);


/**
 * Callback from libspotify, telling us the rootlist is fully synchronized
 * We just print an informational message
 *
 * @param  pc            The playlist container handle
 * @param  userdata      The opaque pointer
 */
extern "C" void container_loaded(sp_playlistcontainer *playlists, void * userdata);




/**
 * Callback from libspotify, saying that a track has been added to a playlist.
 *
 * @param  pl          The playlist handle
 * @param  tracks      An array of track handles
 * @param  num_tracks  The number of tracks in the \c tracks array
 * @param  position    Where the tracks were inserted
 * @param  userdata    The opaque pointer
 */
extern "C" void tracks_added(sp_playlist * pl,
                             sp_track *const * tracks,
                             int num_tracks,
                             int position,
                             void * userdata);


/**
 * Callback from libspotify, telling when tracks have been moved around in a playlist.
 *
 * @param  pl            The playlist handle
 * @param  tracks        An array of track indices
 * @param  num_tracks    The number of tracks in the \c tracks array
 * @param  new_position  To where the tracks were moved
 * @param  userdata      The opaque pointer
 */
extern "C" void tracks_moved(sp_playlist * pl,
                             const int * tracks,
                             int num_tracks,
                             int new_position,
                             void * userdata);

/**
 * Callback from libspotify, saying that a track has been added to a playlist.
 *
 * @param  pl          The playlist handle
 * @param  tracks      An array of track indices
 * @param  num_tracks  The number of tracks in the \c tracks array
 * @param  userdata    The opaque pointer
 */
extern "C" void tracks_removed(sp_playlist * pl, const int * tracks,
                               int num_tracks, void * userdata);


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


#endif
