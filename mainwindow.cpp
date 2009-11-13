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

#include <QApplication>
#include <QPushButton>
#include <QSlider>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QLabel>
#include <QFile>
#include <QTextStream>
#include <QListWidget>
#include <QSplitter>
#include <QPixmap>
#include <QBoxLayout>
#include <QSize>
#include <QStyleFactory>

#include "mainwindow.hpp"
#include "spotworker.hpp"
#include "qlistlistview.hpp"
#include "qlistlistmodel.hpp"
#include "qplaylistmodel.hpp"
#include "qsearchlistmodel.hpp"
#include "qplaylistview.hpp"

#define DEBUGLEVEL 0
#define DEBUG if(DEBUGLEVEL)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    DEBUG printf("MainWindow is being instantiated!\n");

    listListModel_ = new QListListModel(NULL);    this->setWindowTitle("Juniper");
    autoRip_ = false;
    ripFormat_ = SoundSaver::MP3;

    guiUpdater_ = new QTimer;
    spotWorker = SpotWorker::getInstance();
    spotWorker->start(getUsername(), getPassword());
    DEBUG printf("SpotWorker started\n");

    setupGUI();
    DEBUG printf("GUI constructed\n");

    connectSignals();
    DEBUG printf("Signals connected\n");

    guiUpdater_->start(200);

    this->show();

}

void MainWindow::setupGUI()
{

    QWidget *mainWidget = new QWidget;

    mainWidget->setObjectName(QString("mainWidget"));
    //mainWidget->setStyleSheet("QWidget#mainWidget { background-image: url(gfx/background4.gif)}");
    setCentralWidget(mainWidget);

    exitAction_ = new QAction(tr("E&xit"), this);
    exitAction_->setShortcut(tr("Ctrl+Q"));
    exitAction_->setStatusTip(tr("Exit the application"));

    aboutAction_ = new QAction(tr("&About"), this);
    aboutAction_->setStatusTip(tr("About Juniper"));

    fileMenu_ = menuBar()->addMenu(tr("&File"));
    fileMenu_->addAction(exitAction_);
    //These two don't look too good
    //menuBar()->setStyleSheet("QMenuBar { background-image: url(background1.gif)}");
    //fileMenu->setStyleSheet("QMenu { background-image: url(background1.gif)}");

    settingsMenu_ = menuBar()->addMenu(tr("&Settings"));
    formatMenu_ = settingsMenu_->addMenu(tr("Rip format"));
    styleMenu_ = settingsMenu_->addMenu(tr("Window style"));

    formatGroup_ = new QActionGroup(this);

    selectWavAction_ = new QAction(tr("&WAV"), formatGroup_);
    selectFlacAction_ = new QAction(tr("&FLAC"), formatGroup_);
    selectOggAction_ = new QAction(tr("&OGG"), formatGroup_);
    selectMp3Action_ = new QAction(tr("&MP3"), formatGroup_);
    selectWavAction_->setCheckable(true);
    selectFlacAction_->setCheckable(true);
    selectOggAction_->setCheckable(true);
    selectMp3Action_->setCheckable(true);

    selectMp3Action_->setChecked(true);
    formatMenu_->addAction(selectWavAction_);
    formatMenu_->addAction(selectFlacAction_);
    formatMenu_->addAction(selectOggAction_);
    formatMenu_->addAction(selectMp3Action_);

    toggleAutoRipAction_ = new QAction(tr("&AutoRip"), this);
    toggleAutoRipAction_->setCheckable(true);
    toggleAutoRipAction_->setChecked(autoRip_);

    settingsMenu_->addAction(toggleAutoRipAction_);
    settingsMenu_->addMenu(formatMenu_);
    settingsMenu_->addMenu(styleMenu_);

    styleActionGroup_ = new QActionGroup(this);
    foreach(QString styleName, QStyleFactory::keys()){
	QAction *action = new QAction(styleActionGroup_);
	action->setText(tr("%1 Style").arg(styleName));
	action->setData(styleName);
	action->setCheckable(true);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(changeStyle(bool)));
	styleMenu_->addAction(action);
    }
    //checkCurrentStyle();

    aboutMenu_ = menuBar()->addMenu(tr("&Help"));
    aboutMenu_->addAction(aboutAction_);

    searchBox_ = new QLineEdit;
    searchButton_ = new QPushButton(tr("Search"));

    listView_ = new QPlayListView(this);

    QHeaderView *header = listView_->horizontalHeader();
    header->setStretchLastSection(true);
    listView_->setHorizontalHeader(header);
    listView_->horizontalHeader()->resizeSection(0, listView_->horizontalHeader()->frameSize().width()/3);
    listView_->horizontalHeader()->resizeSection(1, listView_->horizontalHeader()->frameSize().width()/3);
    listView_->horizontalHeader()->resizeSection(2, listView_->horizontalHeader()->frameSize().width()/3);

    progressTimeLabel_ = new QLabel("00:00");    
    totalTimeLabel_ = new QLabel("00:00");

    seekSlider_ = new QSlider(Qt::Horizontal);
    seekSlider_->setRange(0,1000);
    seekSlider_->setValue(0);

    buttonPanel_ = new QWidget;
    prevButton_ = new QPushButton(QPixmap("gfx/skip_backward.png"), "");
    playButton_ = new QPushButton(QPixmap("gfx/play.png"), "");
    nextButton_ = new QPushButton(QPixmap("gfx/skip_forward.png"), "");
    netButton_ = new QPushButton(QPixmap("gfx/net.png"), "");

    quitButton_ = new QPushButton(tr("Quit"));
    quitButton_->setFont(QFont("Times", 18, QFont::Bold));

    listSplitter_ = new QSplitter(Qt::Horizontal, this);
    listListView_ = new QListListView(this);

    listSplitter_->addWidget(listListView_);
    listSplitter_->addWidget(listView_);
    listSplitter_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    buttonsLayout_ = new QGridLayout;
    buttonPanel_->setLayout(buttonsLayout_);
    buttonsLayout_->setContentsMargins(0, 0, 0, 0);
    //buttonsLayout->setSpacing(0);
    buttonsLayout_->addWidget(prevButton_, 0, 0);
    buttonsLayout_->addWidget(playButton_, 0, 1);
    buttonsLayout_->addWidget(nextButton_, 0, 2);
    buttonsLayout_->addWidget(progressTimeLabel_, 0, 3);
    buttonsLayout_->addWidget(seekSlider_, 0, 4);
    buttonsLayout_->addWidget(totalTimeLabel_, 0, 5);
    buttonsLayout_->addWidget(netButton_, 0, 6);
    buttonPanel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QGridLayout *layout = new QGridLayout;

    layout->addWidget(searchBox_, 0, 0, 1, 3);
    layout->addWidget(searchButton_, 0, 3, 1, 1);
    layout->addWidget(listSplitter_, 1, 0, 1, 4);
    layout->addWidget(buttonPanel_, 2, 0, 1, 4);

    listSplitter_->setSizes(QList<int>() << 10 << 10000);

    mainWidget->setLayout(layout);
    statusBar()->showMessage(QString("Juniper ready to rock"));
    setMinimumSize(300, 200);
    resize(800, 480);
}

void MainWindow::connectSignals()
{
    connect(guiUpdater_, SIGNAL(timeout()),
	    this, SLOT(updateGui()) );

    connect(spotWorker, SIGNAL(playListsDiscovered(sp_playlistcontainer*)),
	    this, SLOT(updatePlayListList(sp_playlistcontainer*)) );

    connect(netButton_, SIGNAL(clicked()), spotWorker, SLOT(startServer()));

    connect(selectWavAction_, SIGNAL(triggered()), this, SLOT(selectWav()) );
    connect(selectFlacAction_, SIGNAL(triggered()), this, SLOT(selectFlac()) );
    connect(selectOggAction_, SIGNAL(triggered()), this, SLOT(selectOgg()) );
    connect(selectMp3Action_, SIGNAL(triggered()), this, SLOT(selectMp3()) );

    connect(toggleAutoRipAction_, SIGNAL(toggled(bool)),
	    this, SLOT(toggleAutoRip(bool)) );

    connect(exitAction_, SIGNAL(triggered()), this, SLOT(close()));
    connect(aboutAction_, SIGNAL(triggered()), this, SLOT(about()));
    connect(playButton_, SIGNAL(clicked()), this, SLOT(playStop()));

    connect(listListView_, SIGNAL(clicked(const QModelIndex)),
	    this, SLOT(listListClicked(const QModelIndex)) );
    connect(listListView_, SIGNAL(doubleClicked(const QModelIndex)),
	    this, SLOT(listListDoubleClicked(const QModelIndex)) );

    connect( searchBox_, SIGNAL(returnPressed()), this, SLOT(executeSearch()) );
    connect( searchButton_, SIGNAL(clicked()), this, SLOT(executeSearch()) );
    connect( quitButton_, SIGNAL(clicked()), qApp, SLOT(quit()) );

    connect(spotWorker, SIGNAL(searchComplete(sp_search*)), 
	    this, SLOT(searchComplete(sp_search*)) );
    connect(spotWorker, SIGNAL(loggedOut(sp_session*)),
	    this, SLOT(loginFailed()) );

    connect(listView_, SIGNAL(doubleClicked(const QModelIndex)),
	    this, SLOT(songDoubleClicked(const QModelIndex)) );

}

void MainWindow::executeSearch()
{
  DEBUG printf("MainWindow: slot executeSearch() was signaled!\n");

  const QString sstr = searchBox_->text();
  //int length = sstr.length();
  //const QByteArray qba = sstr.toUtf8();
  //int arrsz = qba.size();
  //const char *ss = qba.data();
  //printf("Length of string was %d\n", length);
  //printf("Contents of bytearray: %s\n", ss);
  spotWorker->performSearch(searchBox_->text());
}

void MainWindow::songDoubleClicked(const QModelIndex &index)
{
    DEBUG printf("Song doubleclicked... Trying to play it...\n");

    sp_track* track = listListModel_->getTrack(index);

    DEBUG printf("Loading player...\n");
    spotWorker->loadPlayer(track, autoRip_, ripFormat_);
    DEBUG printf("Playing player...\n");
    playButton_->setText("Stop"); //TODO pål
    spotWorker->playPlayer(true);
}

void MainWindow::playStop()
{
    spotWorker->playPlayer(!spotWorker->isPlaying());
}


//public slot
void MainWindow::searchComplete(sp_search *search)
{
    DEBUG printf("MainWindow: signal searchComplete received!\n");

    int i;

    DEBUG {
	printf("Query          : %s\n", sp_search_query(search));
	printf("Did you mean   : %s\n", sp_search_did_you_mean(search));
	printf("Tracks in total: %d\n", sp_search_total_tracks(search));
    }

    for (i = 0; i < sp_search_num_tracks(search) && i < 40; ++i){

	sp_track *track = sp_search_track(search, i);
	int duration = sp_track_duration(track);
	//sp_album *talbum = sp_track_album(track);
	//const char *albumTitle = sp_album_name(talbum);
	//int artistCount = sp_track_num_artists(track);
	//sp_artist *tartist = sp_track_artist(track, 0);
	//const char *artistName = sp_artist_name(tartist);

	DEBUG printf("  Track \"%s\" [%d:%02d] has %d artist(s), %d%% popularity\n",
	       sp_track_name(track),
	       duration / 60000,
	       (duration / 1000) / 60,
	       sp_track_num_artists(track),
	       sp_track_popularity(track));
    }

    DEBUG printf("mainwindow.cpp: Adding search to listlistmodel\n");
    listListModel_->addSearch(search);
    //TODO: free the previous model before allocating a new
    DEBUG printf("mainwindow.cpp: Creating searchlistmodel\n");
    searchListModel_ = new QSearchListModel(search, listView_);
    DEBUG printf("mainwindow.cpp: Setting searchlistmodel\n");
    listView_->setModel(searchListModel_);

    DEBUG printf("mainwindow.cpp: Executing messy code\n");
    QHeaderView *header = listView_->horizontalHeader();
    header->setStretchLastSection(true);
    listView_->setHorizontalHeader(header);
    listView_->horizontalHeader()->resizeSection(0, listView_->horizontalHeader()->frameSize().width()/3);
    listView_->horizontalHeader()->resizeSection(1, listView_->horizontalHeader()->frameSize().width()/3);
    listView_->horizontalHeader()->resizeSection(2, listView_->horizontalHeader()->frameSize().width()/3);
    listView_->resizeRowsToContents();
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

	progressTimeLabel_->setText(progressText);
	totalTimeLabel_->setText(totalText);
	seekSlider_->setValue(sliderValue);
    }
    else {
	seekSlider_->setValue(0);
    }

    if(spotWorker->isPlaying())
	playButton_->setIcon(QPixmap("gfx/stop.png"));
    else
	playButton_->setIcon(QPixmap("gfx/play.png"));
}


void MainWindow::saveUser(QString username, QString password)
{
    //TODO: save the user
    username = username;
    password = password;
    DEBUG printf("Saving user\n");
}

void MainWindow::updatePlayListList(sp_playlistcontainer *plc)
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
    listListView_->setModel(listListModel_);
}


void MainWindow::selectWav()
{
    toggleFormat(SoundSaver::WAV);
    DEBUG printf("File output format changed to: Wav\n");
}
void MainWindow::selectFlac()
{
    toggleFormat(SoundSaver::FLAC);
    DEBUG printf("File output format changed to: Flac\n");
}
void MainWindow::selectOgg()
{
    toggleFormat(SoundSaver::OGG);
    DEBUG printf("File output format changed to: Ogg\n");
}
void MainWindow::selectMp3()
{
    toggleFormat(SoundSaver::MP3);
    DEBUG printf("File output format changed to: Mp3\n");
}

void MainWindow::toggleFormat(SoundSaver::FileType format)
{
    selectWavAction_->setChecked(false);
    selectFlacAction_->setChecked(false);
    selectOggAction_->setChecked(false);
    selectMp3Action_->setChecked(false);
    switch(format){
    case SoundSaver::WAV:
	selectWavAction_->setChecked(true);
	break;
    case SoundSaver::FLAC:
	selectFlacAction_->setChecked(true);
	break;
    case SoundSaver::OGG:
	selectOggAction_->setChecked(true);
	break;
    case SoundSaver::MP3:
	selectMp3Action_->setChecked(true);
	break;
    }
    ripFormat_ = format;
}

void MainWindow::toggleAutoRip(bool rip)
{
    DEBUG printf("AutoRip toggled\n");
    autoRip_ = rip;
}


void MainWindow::changeStyle(bool checked)
{
    if (!checked)
	return;

    QAction *action = qobject_cast<QAction *>(sender());
    QStyle *style = QStyleFactory::create(action->data().toString());
    Q_ASSERT(style);
    QApplication::setStyle(style);

}


void MainWindow::checkCurrentStyle()
{
    foreach (QAction *action, styleActionGroup_->actions()) {
	QString styleName = action->data().toString();
	QStyle *candidate = QStyleFactory::create(styleName);
	Q_ASSERT(candidate);
	if (candidate->metaObject()->className()
	    == QApplication::style()->metaObject()->className()) {
	    action->trigger();
	    return;
	}
	delete candidate;
    }
}

void MainWindow::listListClicked(const QModelIndex &index)
{
    //TODO: pål
    DEBUG printf("List in playlistlist clicked, index: %d\n", index.row());
    if(listListModel_->isSearchList(index)){
	DEBUG printf("Searchlist clicked\n");
	searchListModel_ = new QSearchListModel(listListModel_->getSearchList(index), listView_);
	DEBUG printf("Setting model\n");
	listView_->setModel(playListModel_);
	DEBUG printf("Model set\n");
    }
    else{
	DEBUG printf("Playlist clicked\n");
	playListModel_ = new QPlayListModel(listListModel_->getPlayList(index), listView_);
	DEBUG printf("Setting model\n");
	listView_->setModel(playListModel_);
	DEBUG printf("Model set\n");
    }
    //connect(listView, SIGNAL(doubleClicked(const QModelIndex)),
    //	this, SLOT(songDoubleClicked(const QModelIndex)) );
}

void MainWindow::listListDoubleClicked(const QModelIndex &/*item*/)
{
    //TODO: pål
    DEBUG printf("Playlist clicked...\n");
}

QString MainWindow::getUsername(void)
{
  QFile file("user.cfg");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    DEBUG printf("Failed to open user file\n");
    return NULL;
  }

  QTextStream in(&file);
  QString line = in.readLine();

  const QByteArray qba = line.toUtf8();
  const char *ss = qba.data();

  DEBUG printf("Returning username: %s\n", ss);

  return line;
}

QString MainWindow::getPassword(void)
{
  QFile file("user.cfg");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    printf("Failed to open user file\n");
    return NULL;
  }

  QString line;

  QTextStream in(&file);
  while (!in.atEnd()) {
    line = in.readLine();
  }

  const QByteArray qba = line.toUtf8();
  const char *ss = qba.data();
  DEBUG printf("Returning password: %s\n", ss);
  return line;
}
