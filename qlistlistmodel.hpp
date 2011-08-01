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

#ifndef QLISTLISTMODEL_H
#define QLISTLISTMODEL_H

#include <QObject>
#include <QAbstractItemModel>

#include <libspotify/api.h>

class QListListModel : public QAbstractItemModel
{
    Q_OBJECT

 public:
    QListListModel(sp_playlistcontainer *plc, QObject *parent = 0);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex&) const;
    int rowCount(const QModelIndex &index) const;
    int columnCount(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    //bool insertRows(int row, sp_track *track);
    sp_playlist *getPlayList(const QModelIndex &index);
    sp_search *getSearchList(const QModelIndex &index);
    sp_track *getTrack(const QModelIndex &index);
    //QVariant headerData(int section, Qt::Orientation orient, int role) const;
    bool isSearchList(const QModelIndex &index);
    void addSearch(sp_search *search);
    int playListCount(void);

 private:
    int selectedIndex;  //bad idea?
    QList<sp_search*> *searchLists;
    sp_playlistcontainer *playLists;
};

#endif
