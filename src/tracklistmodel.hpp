/**
 * @file tracklistmodel.h
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

#ifndef TRACKLISTMODEL_HPP
#define TRACKLISTMODEL_HPP

#include <QObject>
#include <QAbstractTableModel>
#include <libspotify/api.h>

class TrackListModel : public QAbstractTableModel
{
    Q_OBJECT

 public:
    TrackListModel(QObject *parent = 0);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &index) const;
    int columnCount(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    //bool insertRows(int row, sp_track *track);
    sp_track *getTrack(const QModelIndex &index);
    QVariant headerData(int section, Qt::Orientation orient, int role) const;

  public slots:
    void setSearch(sp_search *search);
    void setPlaylist(sp_playlist *pl);

 private:
    sp_search * searchList_;
    sp_playlist * playList_;
    int columns_;
    int currentTrack_;
};

#endif
