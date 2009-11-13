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

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QModelIndex>
#include <spotify/api.h>
#include "qplaylistmodel.hpp"
#include <stdlib.h>
#include <stdio.h>

QPlayListModel::QPlayListModel(sp_playlist *pl, QObject *parent)
  : QAbstractTableModel(parent)
{
  this->playList = pl;
  this->columns = 3;
}

QVariant QPlayListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //printf("DEBUG: headerdata requested. section was: %d\n", section);

    if (role != Qt::DisplayRole)
	return QVariant();

    if(orientation==Qt::Horizontal){
	switch(section){
	case 0:
	    return QVariant("Title");
	case 1:
	    return QVariant("Artist");
	case 2:
	    return QVariant("Album");
	default:
	    return QVariant("Error");
	}
    }
    else {
	return section+1;
    }
}

QModelIndex QPlayListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //printf("DEBUG: model->index() called\n");
    return createIndex(row, column);
}

int QPlayListModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    //printf("This: %p\n", this);
    //printf("List: %p\n", this->list);
    int count = sp_playlist_num_tracks(this->playList);
    //printf("Tracks in this list: %d\n", count);
    return count;
}

int QPlayListModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    //printf("columnCount requested\n");
    return this->columns;
}

int QPlayListModel::getTrackCount()
{
    int count = sp_playlist_num_tracks(this->playList);
    return count;
}

sp_track *QPlayListModel::getTrack(const QModelIndex &index)
{
    printf("getTrack called\n");
    int row = index.row();
    return sp_playlist_track(this->playList, row);
}

QVariant QPlayListModel::data(const QModelIndex &index, int role) const
{

    //printf("Role: %d\n", role);

    if(role==Qt::DisplayRole){

	int row = index.row();
	int column = index.column();
	//printf("Requesting DisplayRole data for row %d, col %d\n", row, column);
	sp_track *track = sp_playlist_track(this->playList, row);
	//if(!track)
	    //printf("Epic fail. sp_playlist_track returned null-pointer\n");
	int artistCount = sp_track_num_artists(track);

	switch(column){
	case 0: {
	    const char *trackName = sp_track_name(track);
	    return QString().fromUtf8(trackName);
	}
	case 1: {
	    if(artistCount<=0){
		//printf("This track has %d artists\n", artistCount);
		return QString("");
	    }
	    //printf("Attempting to extract artist\n");
	    sp_artist *tartist = sp_track_artist(track, 0);
	    //if(!tartist)
		//printf("Epic fail. sp_track_artist returned null-pointer\n");
	    const char *artistName = sp_artist_name(tartist);
	    return QString().fromUtf8(artistName);
	}
	case 2: {
	    sp_album *talbum = sp_track_album(track);
	    if(!talbum){
		return QString("");
	    }
	    const char *albumName = sp_album_name(talbum);
	    return QString().fromUtf8(albumName);
	}
	default: {
	    return QString("Error!");
	}
	}
    }
    else{
	return QVariant();
    }

    //int duration = sp_track_duration(track);
    //printf("Duration: %d\n", duration);
    //sp_album *talbum = sp_track_album(track);
    //char *albumTitle = sp_album_name(talbum);
    //int artistCount = sp_track_num_artists(track);

    //return toReturn;

    /*
    const char *foo = "bar";
    
    int column = index.column();
    */

    /*
    switch(column){
    case 0:
	QString cellText(artistName);
		return cellText;
    }
    */
    return "Error...";
    //QVariant data = qVariantFromValue(this->list.at(index.row()));
}

bool QPlayListModel::insertRows(int row, sp_track *track)
{
    //QModelIndex invalid;
    //QAbstractTableModel::beginInsertRows(invalid, row, row);
    //QAbstractTableModel::endInsertRows();
    Q_UNUSED(track);
    Q_UNUSED(row);
    printf("insertRows called\n");
    return true;
}
