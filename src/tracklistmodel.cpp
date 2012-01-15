/**
 * @file tracklistmodel.cpp
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

#include "tracklistmodel.hpp"

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QModelIndex>
#include <libspotify/api.h>
#include <stdlib.h>
#include <QDebug>
#include <cassert>

#define DEBUGLEVEL 0
#define DEBUG if(DEBUGLEVEL)

TrackListModel::TrackListModel(QObject *parent)
  : QAbstractTableModel(parent)
{
  this->columns_ = 3;
  searchList_ = NULL;
  playList_ = NULL;
}

QVariant TrackListModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (role != Qt::DisplayRole)
        return QVariant();

    if(orientation==Qt::Horizontal){
        //printf("DEBUG: headerdata requested. section was: %d\n", section);
        switch(section){
            case 0:
                return QVariant(tr("Title"));
            case 1:
                return QVariant(tr("Artist"));
            case 2:
                return QVariant(tr("Album"));
            case 3:
                return QVariant(tr("Duration"));
            default:
                return QVariant(tr("Error"));
        }
    }
    else {
        return section+1;
    }
}

void TrackListModel::setSearch(sp_search *search)
{
    beginResetModel();

    playList_ = NULL;
    qDebug() << "Query          :" << sp_search_query(search) << "\n"
             << "Did you mean   :" << sp_search_did_you_mean(search) << "\n"
             << "Tracks in total:" << sp_search_total_tracks(search);


    /*
    for (int i = 0; i < sp_search_num_tracks(search) && i < 40; ++i){
        sp_track *track = sp_search_track(search, i);
        int duration = sp_track_duration(track);

        // FIXME, unicode breaks
        qDebug() << trUtf8("Track \"%1\" [%2:%3] has %4 artist(s), %5 popularity")
            .arg(sp_track_name(track))
            .arg(duration/60000)
            .arg((duration/1000)/60, 2)
            .arg(sp_track_num_artists(track))
            .arg(sp_track_popularity(track));

        printf("  Track \"%s\" [%d:%02d] has %d artist(s), %d%% popularity\n",
               sp_track_name(track),
               duration / 60000,
               (duration / 1000) / 60,
               sp_track_num_artists(track),
               sp_track_popularity(track));
    }
    */

    searchList_ = search;
    endResetModel();
}

void TrackListModel::setPlaylist(sp_playlist *pl)
{
    beginResetModel();

    searchList_ = NULL;


    playList_ = pl;
    endResetModel();
}

QModelIndex TrackListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column);
}

int TrackListModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);

    int rowCount = 0;
    if (searchList_ != NULL)
        rowCount = sp_search_num_tracks(searchList_);

    if (playList_ != NULL) {
        rowCount = sp_playlist_num_tracks(playList_);
    }

    return rowCount;
}

int TrackListModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return columns_;
}

sp_track * TrackListModel::getTrack(const QModelIndex &index)
{
    int row = index.row();

    if (searchList_ != NULL)
        return sp_search_track(searchList_, row);

    if (playList_ != NULL)
        return sp_playlist_track(playList_, row);

    return NULL;
}

QVariant TrackListModel::data(const QModelIndex &index, int role) const
{
    if(role==Qt::DisplayRole){
        int row = index.row();
        int column = index.column();

        sp_track *track;

        if (searchList_ != NULL) {
            track = sp_search_track(searchList_, row);
        }
        else if (playList_ != NULL) {
            track = sp_playlist_track(playList_, row);
        }
        else {
            return QVariant();
        }

        switch(column){
            case 0: {
                const char * trackName = sp_track_name(track);
                if (trackName[0] == '\0') {
                    return QString().fromUtf8("Loading...");
                }
                else {
                    return QString().fromUtf8(trackName);
                }
            }
            case 1: {
                int numArtists = sp_track_num_artists(track);
                //If no metadata is available for the track yet, sp_track_num_artists returns 0
                if (numArtists == 0) {
                    return QString().fromUtf8("");
                }
                else {
                    sp_artist * tartist = sp_track_artist(track, 0);
                    const char *artistName = sp_artist_name(tartist);
                    return QString().fromUtf8(artistName);
                }
            }
            case 2: {
                sp_album * talbum = sp_track_album(track);
                //If no metadata is available for the track yet, sp_track_num_artists returns 0
                if (talbum == 0) {
                    return QString().fromUtf8("");
                }
                else {
                    const char * albumName = sp_album_name(talbum);
                    return QString().fromUtf8(albumName);
                }
            }
            case 3: {
                int duration = sp_track_duration(track);
                return duration;
            }
            default: {
                return QVariant();
            }
        }
    }
    else{
        return QVariant();
    }

    return QVariant();
}

/*
bool TrackListModel::insertRows(int row, sp_track *track)
{
    //QModelIndex invalid;
    //QAbstractTableModel::beginInsertRows(invalid, row, row);
    //QAbstractTableModel::endInsertRows();
    Q_UNUSED(track);
    Q_UNUSED(row);
    return true;
}
*/
