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
#include <QAbstractItemModel>
#include <QPixmap>

#include <libspotify/api.h>
#include "qlistlistmodel.hpp"
#include <stdlib.h>
#include <stdio.h>

#define DEBUGLEVEL 0
#define DEBUG if(DEBUGLEVEL)

QListListModel::QListListModel(QObject *parent)
  : QAbstractItemModel(parent)
{
  playLists = NULL;
  searchLists = new QList<sp_search*>;
}

void QListListModel::setPlayLists(sp_playlistcontainer *plc)
{
  beginResetModel();

  int listCount = sp_playlistcontainer_num_playlists(plc);
  for(int i = 0; i < listCount; i++){
    sp_playlist *pl = sp_playlistcontainer_playlist(plc, i);
    const char *listName = sp_playlist_name(pl);
    if (sp_playlist_is_loaded(pl)){
      printf("List %i/%i: %s\n", i, listCount, listName);
    } else
      printf("List %i not loaded\n", i);
  }

  playLists = plc;

  endResetModel();
}

/*
QVariant QListListModel::headerData(int section, Qt::Orientation orientation, int role) const
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
*/

QModelIndex QListListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //    printf("DEBUG: model->index() called\n");
    return createIndex(row, column);
}

int QListListModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    //printf("This: %p\n", this);
    //printf("List: %p\n", this->list);
    int count = 0;
    //TODO: add searchLists and playLists
    if(this->playLists)
        count += sp_playlistcontainer_num_playlists(this->playLists);;
    //printf("Count: %d\n", count);
    return count;
}

int QListListModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 1;
}

sp_playlist *QListListModel::getPlayList(const QModelIndex &index)
{
    // TODO: DANGER! playList may be NULL, though this function
    // should in THEORY never be called if it is...
    int row = index.row();
    selectedIndex = this->searchLists->size() + row;
    return sp_playlistcontainer_playlist(this->playLists, row);
}

sp_search *QListListModel::getSearchList(const QModelIndex &index)
{
    int row = index.row();
    if(row>(searchLists->size()-1)){
        DEBUG printf("Serirorus error occured. Bounds check failed in listlistmodel.\n");
        return this->searchLists->at(0);
    }
    else{
        return this->searchLists->at(row);
    }
}

sp_track *QListListModel::getTrack(const QModelIndex &index)
{
    //TODO: add extra bounds checking on index.row()
    DEBUG printf("QListListModel: getTrack()\n");
    if(selectedIndex < searchLists->size()){
        return sp_search_track(searchLists->at(selectedIndex), index.row());
    }
    else if ((selectedIndex - searchLists->size()) < playListCount()){
        sp_playlist *pl = sp_playlistcontainer_playlist(playLists, selectedIndex - searchLists->size());
        return sp_playlist_track(pl, index.row());
    }
    else{
        DEBUG printf("FATAL ERROR: bounds check failed in QListListModel::getTrack\n");
        DEBUG printf("index: %d, searchLists->size = %d, playListCount() = %d\n", selectedIndex, searchLists->size(), playListCount());
    }
    return NULL;
}

int QListListModel::playListCount(void)
{
    if(playLists)
        return sp_playlistcontainer_num_playlists(this->playLists);
    else
        return 0;
}

QVariant QListListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    sp_playlist *pl = sp_playlistcontainer_playlist(this->playLists, row);

    if (role == Qt::DisplayRole){
        return QString(QString::fromUtf8(sp_playlist_name(pl)));
    } else if (role == Qt::DecorationRole){
        return QPixmap(":/gfx/album.svg").scaled(64, 64);
        /*
        // TODO request image through spotify helpers
        byte imageId[20];
        if (sp_playlist_get_image(pl, &imageId)){
            sp_image *image = sp_image_create(
        }
        */
    } else {
        return QVariant();
    }

    return "Error...";
}


/*
bool QListListModel::insertRows(int row, sp_track *track)
{
    //QModelIndex invalid;
    //QAbstractTableModel::beginInsertRows(invalid, row, row);
    //QAbstractTableModel::endInsertRows();
    Q_UNUSED(track);
    Q_UNUSED(row);
    return true;
}
*/

void QListListModel::addSearch(sp_search *search)
{
    DEBUG printf("QListListModel::addSearch: appending searchlist\n");
    this->searchLists->append(search);
    DEBUG printf("QListListModel::addSearch: setting selectedIndex\n");
    this->selectedIndex = searchLists->size()-1;
}

bool QListListModel::isSearchList(const QModelIndex &index)
{
    if(index.row() < searchLists->size())
        return true;
    else
        return false;
}

QModelIndex QListListModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}
