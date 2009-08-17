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
#include <QAction>
#include <QMenu>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <spotify/api.h>
#include "qplaylistview.hpp"
#include "qplaylistmodel.hpp"
#include <stdlib.h>

QPlayListView::QPlayListView(QWidget *parent)
  : QTableView(parent)
{

     cutAct = new QAction(tr("Save as &WAV"), this);
     //cutAct->setShortcuts(QKeySequence::Cut);
     cutAct->setStatusTip(tr("Save this song as a WAV file"));
     //connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

     copyAct = new QAction(tr("Save as &FLAC"), this);
     //copyAct->setShortcut(tr("Ctrl+C"));
     copyAct->setStatusTip(tr("Save this song as a FLAC file"));
     //connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

     pasteAct = new QAction(tr("Save as &OGG"), this);
     //pasteAct->setShortcuts(QKeySequence::Paste);
     pasteAct->setStatusTip(tr("Save this song as a OGG file"));
     //connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));


     //QHeaderView *header = horizontalHeader();

     this->setShowGrid(false);
     this->verticalHeader()->setVisible(false);
     this->setSelectionBehavior(QAbstractItemView::SelectRows);
     this->setAlternatingRowColors(true);
     
}

void QPlayListView::contextMenuEvent(QContextMenuEvent * /*event*/)
{
    /*
    QMenu menu(this);
    menu.addAction(cutAct);
    menu.addAction(copyAct);
    menu.addAction(pasteAct);
    menu.exec(event->globalPos());
    */
}
