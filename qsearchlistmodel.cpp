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
#include "qsearchlistmodel.hpp"
#include <stdlib.h>

QSearchListModel::QSearchListModel(sp_search *pl, QObject *parent)
  : QAbstractTableModel(parent)
{
  this->searchList_ = pl;
  this->columns_ = 3;
}

QVariant QSearchListModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (role != Qt::DisplayRole)
	return QVariant();

    if(orientation==Qt::Horizontal){
	//printf("DEBUG: headerdata requested. section was: %d\n", section);
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

QModelIndex QSearchListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //    printf("DEBUG: model->index() called\n");
    return createIndex(row, column);
}

int QSearchListModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    //printf("This: %p\n", this);
    //printf("List: %p\n", this->list);
    int count = sp_search_num_tracks(this->searchList_);
    //printf("Count: %d\n", count);
    return count;
}

int QSearchListModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return columns_;
}

sp_track *QSearchListModel::getTrack(const QModelIndex &index)
{
    int row = index.row();
    return sp_search_track(this->searchList_, row);
}

QVariant QSearchListModel::data(const QModelIndex &index, int role) const
{

    //printf("Role: %d\n", role);

    if(role==Qt::DisplayRole){

	int row = index.row();
	int column = index.column();
	//printf("requesting row %d\n", row);
	sp_track *track = sp_search_track(this->searchList_, row);

	switch(column){
	case 0: {
	    const char *trackName = sp_track_name(track);
	    return QString().fromUtf8(trackName);
	}
	case 1: {
	    sp_artist *tartist = sp_track_artist(track, 0);
	    const char *artistName = sp_artist_name(tartist);
	    return QString().fromUtf8(artistName);
	}
	case 2: {
	    sp_album *talbum = sp_track_album(track);
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

/*
bool QSearchListModel::insertRows(int row, sp_track *track)
{
    //QModelIndex invalid;
    //QAbstractTableModel::beginInsertRows(invalid, row, row);
    //QAbstractTableModel::endInsertRows();
    Q_UNUSED(track);
    Q_UNUSED(row);
    return true;
}
*/
