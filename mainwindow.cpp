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

#include <QStyleFactory>
#include <QMessageBox>

#include "mainwindow.hpp"
#include "spotworker.hpp"
#include "qlistlistview.hpp"
#include "qlistlistmodel.hpp"
#include "qplaylistmodel.hpp"
#include "qsearchlistmodel.hpp"
#include "qplaylistview.hpp"

#define DEBUGLEVEL 1
#define DEBUG if(DEBUGLEVEL)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug() << "MainWindow is being instantiated!";
    setupUi(this);

    formatGroup_ = new QActionGroup(this);
    formatGroup_->addAction(selectWavAction);
    formatGroup_->addAction(selectOggAction);
    formatGroup_->addAction(selectFlacAction);
    formatGroup_->addAction(selectMp3Action);

/*
    listView->horizontalHeader()->resizeSection(0, listView->horizontalHeader()->frameSize().width()/3);
    listView->horizontalHeader()->resizeSection(1, listView->horizontalHeader()->frameSize().width()/3);
    listView->horizontalHeader()->resizeSection(2, listView->horizontalHeader()->frameSize().width()/3);
*/

    listSplitter->setSizes(QList<int>() << 10 << 10000);

    bool initialized=settings.value("initialized").toBool();

    if (!initialized){
        qDebug() << "Setting initial settings...";
        settings.setValue("initialized", true);
        settings.setValue("ripFormat", SoundSaver::MP3);
        settings.setValue("autoRip", false);
    }

    listListModel_ = new QListListModel(NULL);    this->setWindowTitle("Juniper");
    ripFormat_ = static_cast<SoundSaver::FileType>(settings.value("ripFormat").toUInt());

    guiUpdater_ = new QTimer;
    spotWorker = SpotWorker::getInstance();
    spotWorker->start(settings.value("username").toString(),
                      settings.value("password").toString());
    qDebug() << "SpotWorker started";

    connectSignals();
    qDebug() << "Signals connected";

    guiUpdater_->start(200);

    this->show();

    statusBar()->showMessage(QString("Juniper ready to rock"));
    setMinimumSize(300, 200);
    resize(800, 480);
}

void MainWindow::connectSignals()
{
    connect(guiUpdater_, SIGNAL(timeout()),
	    this, SLOT(updateGui()) );

    connect(spotWorker, SIGNAL(playlistAdded(sp_playlistcontainer*)),
	    this, SLOT(updatePlaylistList(sp_playlistcontainer*)) );

    //connect(seekSlider, SIGNAL(sliderMoved(int)), 
    //        spotWorker, SLOT(seekPlayer(int)));
    connect(netButton, SIGNAL(clicked()), spotWorker, SLOT(startServer()));

    connect(listListView, SIGNAL(clicked(const QModelIndex)),
	    this, SLOT(listListClicked(const QModelIndex)) );
    connect(listListView, SIGNAL(doubleClicked(const QModelIndex)),
	    this, SLOT(listListDoubleClicked(const QModelIndex)) );

    connect(spotWorker, SIGNAL(searchComplete(sp_search*)), 
	    this, SLOT(searchComplete(sp_search*)) );
    connect(spotWorker, SIGNAL(loggedOut(sp_session*)),
	    this, SLOT(loginFailed()) );

    connect(listView, SIGNAL(doubleClicked(const QModelIndex)),
	    this, SLOT(songDoubleClicked(const QModelIndex)) );
}

void MainWindow::executeSearch()
{
  qDebug() << "MainWindow: slot executeSearch() was signaled!";

  //printf("SIZE: %d\n", listView->getModel()->playListCount();
  qDebug() << "calling performsearch";
  spotWorker->performSearch(searchBox->currentText());
}

void MainWindow::songDoubleClicked(const QModelIndex &index)
{
    qDebug() << "Song doubleclicked... Trying to play it...";

    sp_track* track = listListModel_->getTrack(index);

    qDebug() << "Loading player...";
    spotWorker->loadPlayer(track, settings.value("autoRip").toBool(), ripFormat_);
    qDebug() << "Playing player...";
    spotWorker->playPlayer(true);
}

void MainWindow::playStop()
{
    spotWorker->playPlayer(!spotWorker->isPlaying());
}


//public slot
void MainWindow::searchComplete(sp_search *search)
{
    qDebug() << "MainWindow: signal searchComplete received!";

    int i;

    qDebug() << "Query          :" << sp_search_query(search) << "\n"
             << "Did you mean   :" << sp_search_did_you_mean(search) << "\n"
             << "Tracks in total:" << sp_search_total_tracks(search);

    for (i = 0; i < sp_search_num_tracks(search) && i < 40; ++i){

	sp_track *track = sp_search_track(search, i);
	int duration = sp_track_duration(track);
	//sp_album *talbum = sp_track_album(track);
	//const char *albumTitle = sp_album_name(talbum);
	//int artistCount = sp_track_num_artists(track);
	//sp_artist *tartist = sp_track_artist(track, 0);
	//const char *artistName = sp_artist_name(tartist);

        qDebug() << "  Track" << sp_track_name(track) << "["
                 << duration / 60000 <<":"<< (duration / 1000) / 60 
                 << "] has " << sp_track_num_artists(track) << " artist(s), "
                 << sp_track_popularity(track) << "% popularity";
	DEBUG printf("  Track \"%s\" [%d:%02d] has %d artist(s), %d%% popularity\n",
	       sp_track_name(track),
	       duration / 60000,
	       (duration / 1000) / 60,
	       sp_track_num_artists(track),
	       sp_track_popularity(track));
    }

    qDebug() << "mainwindow.cpp: Adding search to listlistmodel";
    listListModel_->addSearch(search);
    //TODO: free the previous model before allocating a new
    qDebug() << "mainwindow.cpp: Creating searchlistmodel";
    searchlistModel_ = new QSearchListModel(search, listView);
    qDebug() << "mainwindow.cpp: Setting searchlistmodel";
    listView->setModel(searchlistModel_);

    qDebug() << "mainwindow.cpp: Executing messy code";
    QHeaderView *header = listView->horizontalHeader();
    header->setStretchLastSection(true);
    listView->setHorizontalHeader(header);
    listView->horizontalHeader()->resizeSection(0, listView->horizontalHeader()->frameSize().width()/3);
    listView->horizontalHeader()->resizeSection(1, listView->horizontalHeader()->frameSize().width()/3);
    listView->horizontalHeader()->resizeSection(2, listView->horizontalHeader()->frameSize().width()/3);
    listView->resizeRowsToContents();
    //TODO: we might have to free the search pointer or some shit
}


void MainWindow::about()
{
    //infoLabel->setText(tr("Invoked <b>Help|About</b>"));
    QMessageBox::about(this, tr("About Juniper"),
		       tr("<b>Juniper</b> is the most epic spotify client ever "
			  "<br>Creds to my peeps"));
}

void MainWindow::loginFailed()
{
    //infoLabel->setText(tr("Invoked <b>Help|About</b>"));
    QMessageBox::about(this, tr("Error!"),
		       tr("<b>Juniper</b> failed to connect to Spotify<br>"
			  "Correct username/password?"));
}

void MainWindow::updateGui()
{
    int progress = spotWorker->getProgress();
    int total = spotWorker->getSongLength();

    if(total>0){
	int sliderValue = progress*1000/total;

	//Construct progress string
	QString progressText = QString().setNum( (progress/1000)%60 );
	if(progressText.length()<2)
	    progressText.prepend("0");
	progressText.prepend(QString().setNum( (progress/1000)/60 ) + ":");

	//Construct totalTime string
	QString totalText = QString().setNum( (total/1000)%60 );
	if(totalText.length()<2)
	    totalText.prepend("0");
	totalText.prepend(QString().setNum( (total/1000)/60 ) + ":");

	progressTimeLabel->setText(progressText);
	totalTimeLabel->setText(totalText);
	seekSlider->setValue(sliderValue);
    }
    else {
	seekSlider->setValue(0);
    }

    if(spotWorker->isPlaying())
	playButton->setIcon(QPixmap(":gfx/stop.png"));
    else
	playButton->setIcon(QPixmap(":gfx/play.png"));
}


void MainWindow::updatePlaylistList(sp_playlistcontainer *plc)
{
    //int listCount = sp_playlistcontainer_num_playlists(plc);
    /*
      for(int i = 0; i < listCount; i++){
      sp_playlist *pl = sp_playlistcontainer_playlist(plc, i);
      const char *listName = sp_playlist_name(pl);
    }
    */
    if(!listListModel_)
	listListModel_ = new QListListModel(plc);
    listListView->setModel(listListModel_);
    listListView->update();
    qDebug() << "MainWindow::updatePlaylistList: list model size: " 
             << listListModel_->playListCount();
}

void MainWindow::toggleRipFormat()
{
    SoundSaver::FileType format;

    if (selectWavAction->isChecked())
        format = SoundSaver::WAV;
    else if (selectFlacAction->isChecked())
        format = SoundSaver::FLAC;
    else if (selectOggAction->isChecked())
        format = SoundSaver::OGG;
    else 
        format = SoundSaver::MP3;

    ripFormat_ = format;
    settings.setValue("ripFormat", format);
}

void MainWindow::toggleAutoRip(bool rip)
{
    qDebug() << "AutoRip toggled";
    settings.setValue("autoRip", rip);
}

void MainWindow::listListClicked(const QModelIndex &index)
{
    //TODO: pål
    qDebug() << "List in playlistlist clicked, index: " << index.row();
    if(listListModel_->isSearchList(index)){
        qDebug() << "Searchlist clicked...";
	searchlistModel_ = new QSearchListModel(listListModel_->getSearchList(index), listView);
	qDebug() << "Setting model";
	listView->setModel(playlistModel_);
	qDebug() << "Model set";
    }
    else{
        qDebug() << "Playlist clicked...";
	playlistModel_ = new QPlayListModel(listListModel_->getPlayList(index), listView);
	qDebug() << "Setting model";
	listView->setModel(playlistModel_);
	qDebug() << "Model set";
    }
    //connect(listView, SIGNAL(doubleClicked(const QModelIndex)),
    //	this, SLOT(songDoubleClicked(const QModelIndex)) );
}

void MainWindow::listListDoubleClicked(const QModelIndex &/*item*/)
{
    //TODO: pål
    qDebug() << "Playlist doubleclicked...";
}

