/**
 * @file qlistlistmodel.cpp
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

#include "qlistlistmodel.hpp"

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QPixmap>

#include <libspotify/api.h>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>

#define DEBUGLEVEL 0
#define DEBUG if(DEBUGLEVEL)

QListListModel::QListListModel(QObject *parent)
    : QAbstractItemModel(parent)
    , selectedIndex_(0)
    , playLists_(NULL)
{
}

void QListListModel::setPlayLists(sp_playlistcontainer *plc)
{
    DEBUG printf("%s::%s\n", __FILE__, __func__);
    beginResetModel();
    playLists_ = plc;
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
    if(playLists_)
        count += sp_playlistcontainer_num_playlists(playLists_);
    //printf("Count: %d\n", count);
    return count;
}

int QListListModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return 1;
}

sp_playlist * QListListModel::getPlayList(const QModelIndex &index)
{
    // TODO: DANGER! playList may be NULL, though this function
    // should in THEORY never be called if it is...
    int row = index.row();
    selectedIndex_ = searchLists_.size() + row;
    assert(playLists_);
    return sp_playlistcontainer_playlist(playLists_, row);
}

sp_search *QListListModel::getSearchList(const QModelIndex &index)
{
    int row = index.row();
    if(row>(searchLists_.size()-1)){
        DEBUG printf("Serirorus error occured. Bounds check failed in listlistmodel.\n");
        return searchLists_.at(0);
    }
    else{
        return searchLists_.at(row);
    }
}

sp_track *QListListModel::getTrack(const QModelIndex &index)
{
    //TODO: add extra bounds checking on index.row()
    DEBUG printf("QListListModel: getTrack()\n");
    if(selectedIndex_ < searchLists_.size()){
        return sp_search_track(searchLists_.at(selectedIndex_), index.row());
    }
    else if ((selectedIndex_ - searchLists_.size()) < playListCount()){
        sp_playlist *pl = sp_playlistcontainer_playlist(playLists_,
                                                        selectedIndex_ - searchLists_.size());
        return sp_playlist_track(pl, index.row());
    }
    else{
        DEBUG printf("FATAL ERROR: bounds check failed in QListListModel::getTrack\n");
        DEBUG printf("index: %d, searchLists->size = %d, playListCount() = %d\n",
                     selectedIndex_, searchLists_.size(), playListCount());
    }
    return NULL;
}

int QListListModel::playListCount(void)
{
    if(playLists_)
        return sp_playlistcontainer_num_playlists(playLists_);
    else
        return 0;
}

QVariant QListListModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    sp_playlist *pl = sp_playlistcontainer_playlist(playLists_, row);

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
    searchLists_.append(search);
    DEBUG printf("QListListModel::addSearch: setting selectedIndex\n");
    selectedIndex_ = searchLists_.size()-1;
}

bool QListListModel::isSearchList(const QModelIndex &index)
{
    if(index.row() < searchLists_.size())
        return true;
    else
        return false;
}

QModelIndex QListListModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}
