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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPushButton>
#include <QLineEdit>
#include <QObject>
#include <QWidget>
#include <QTableView>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
//#include <QAbstractTableModel>

#include <spotify/api.h>
#include "spotworker.hpp"
#include "qlistlistview.hpp"
#include "qlistlistmodel.hpp"
#include "qplaylistview.hpp"
#include "qplaylistmodel.hpp"
#include "qsearchlistmodel.hpp"

class MainWindow : public QMainWindow {

    Q_OBJECT

 public:
    MainWindow(QWidget *parent = 0);
    SpotWorker *spotWorker; //TODO: make private!
    static QString getUsername(void);
    static QString getPassword(void);

 private:

    //GUI stuff
    QLineEdit *searchBox;
    QPushButton *searchButton;
    QWidget *buttonPanel;
    QPushButton *playButton;
    QPushButton *nextButton;
    QPushButton *prevButton;
    QPushButton *netButton;
    QPushButton *quitButton;
    QSplitter *listSplitter;
    QMenu *formatMenu;
    QMenu *styleMenu;
    QMenu *fileMenu;
    QMenu *settingsMenu;
    QMenu *helpMenu;
    QMenu *aboutMenu;
    QSlider *seekSlider;
    QLabel *progressTimeLabel;
    QLabel *totalTimeLabel;
    QTimer *guiUpdater;

    //models and structure
    QListListView *listListView;
    QListListModel *listListModel;
    QPlayListView *listView;

    QPlayListModel *nowPlayingPlaylist;
    QSearchListModel *nowPlayingSearch;

    QPlayListModel *playListModel;
    QSearchListModel *searchListModel;

    QAction *aboutAction;
    QAction *exitAction;
    bool autoRip;
    SoundSaver::FileType ripFormat;
    QActionGroup *formatGroup;
    QActionGroup *styleActionGroup;
    QAction *selectWavAction;
    QAction *selectFlacAction;
    QAction *selectOggAction;
    QAction *selectMp3Action;
    QAction *toggleAutoRipAction;

    //functions
    void setupGUI();
    void connectSignals();
    void toggleFormat(SoundSaver::FileType);
    void saveUser(QString user, QString pass);
    void checkCurrentStyle(void);

 signals:
    void someSignal();

 public slots:
    void executeSearch();
    void searchComplete(sp_search *search);
    void songDoubleClicked(const QModelIndex &index);
    void listListClicked(const QModelIndex &index);
    void listListDoubleClicked(const QModelIndex &index);
    void loginFailed(void);
    void about();
    void updateGui();
    void selectWav();
    void selectFlac();
    void selectOgg();
    void selectMp3();
    void toggleAutoRip(bool);
    void playStop();
    void changeStyle(bool checked);
    void updatePlayListList(sp_playlistcontainer *plc);

};


class QMainWidget : public QWidget {
    
    Q_OBJECT
	
};

#endif
