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
#include <QGridLayout>

#include <libspotify/api.h>
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
    QLineEdit *searchBox_;
    QPushButton *searchButton_;
    QWidget *buttonPanel_;
    QGridLayout *buttonsLayout_;
    QPushButton *playButton_;
    QPushButton *nextButton_;
    QPushButton *prevButton_;
    QPushButton *netButton_;
    QPushButton *quitButton_;
    QSplitter *listSplitter_;
    QMenu *formatMenu_;
    QMenu *styleMenu_;
    QMenu *fileMenu_;
    QMenu *settingsMenu_;
    QMenu *helpMenu_;
    QMenu *aboutMenu_;
    QSlider *seekSlider_;
    QLabel *progressTimeLabel_;
    QLabel *totalTimeLabel_;
    QTimer *guiUpdater_;

    //models and structure
    QListListView *listListView_;
    QListListModel *listListModel_;
    QPlayListView *listView_;

    QPlayListModel *nowPlayingPlaylist_;
    QSearchListModel *nowPlayingSearch_;

    QPlayListModel *playlistModel_;
    QSearchListModel *searchlistModel_;

    QAction *aboutAction_;
    QAction *exitAction_;
    bool autoRip_;
    SoundSaver::FileType ripFormat_;
    QActionGroup *formatGroup_;
    QActionGroup *styleActionGroup_;
    QAction *selectWavAction_;
    QAction *selectFlacAction_;
    QAction *selectOggAction_;
    QAction *selectMp3Action_;
    QAction *toggleAutoRipAction_;

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
    void updatePlaylistList(sp_playlistcontainer *plc);

};


class QMainWidget : public QWidget {
    
    Q_OBJECT
	
};

#endif
