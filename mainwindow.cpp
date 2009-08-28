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

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{

    printf("MainWindow is being instantiated!\n");

    this->setWindowTitle("Juniper");

    autoRip = false;
    ripFormat = SoundSaver::MP3;    

    spotWorker = SpotWorker::getInstance();
    spotWorker->start(getUsername(), getPassword());

    printf("SpotWorker started\n");

    setupGUI();

    printf("GUI constructed\n");

    QTimer *guiUpdater = new QTimer;

    connect(guiUpdater, SIGNAL(timeout()),
	    this, SLOT(updateGui()) );

    connect(spotWorker, SIGNAL(playListsDiscovered(sp_playlistcontainer*)),
	    this, SLOT(updatePlayListList(sp_playlistcontainer*)) );

    connect(netButton, SIGNAL(clicked()), spotWorker, SLOT(startServer()));

    connect(selectWavAction, SIGNAL(triggered()), this, SLOT(selectWav()) ); 
    connect(selectFlacAction, SIGNAL(triggered()), this, SLOT(selectFlac()) ); 
    connect(selectOggAction, SIGNAL(triggered()), this, SLOT(selectOgg()) ); 
    connect(selectMp3Action, SIGNAL(triggered()), this, SLOT(selectMp3()) );     

    connect(toggleAutoRipAction, SIGNAL(toggled(bool)),
	    this, SLOT(toggleAutoRip(bool)) );

    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(playButton, SIGNAL(clicked()), this, SLOT(playStop()));

    connect(listListView, SIGNAL(clicked(const QModelIndex)),
	    this, SLOT(listListClicked(const QModelIndex)) );
    connect(listListView, SIGNAL(doubleClicked(const QModelIndex)),
	    this, SLOT(listListDoubleClicked(const QModelIndex)) );

    connect( searchBox, SIGNAL(returnPressed()), this, SLOT(executeSearch()) );
    connect( searchButton, SIGNAL(clicked()), this, SLOT(executeSearch()) );
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );

    connect(spotWorker, SIGNAL(searchComplete(sp_search*)), 
	    this, SLOT(searchComplete(sp_search*)) );
    connect(spotWorker, SIGNAL(loggedOut(sp_session*)),
	    this, SLOT(loginFailed()) );

    connect(listView, SIGNAL(doubleClicked(const QModelIndex)),
	    this, SLOT(songDoubleClicked(const QModelIndex)) );

    printf("Signals connected\n");
    guiUpdater->start(200);

    this->show();

}

void MainWindow::setupGUI()
{
    
    QWidget *mainWidget = new QWidget;
    
    mainWidget->setObjectName(QString("mainWidget"));
    //mainWidget->setStyleSheet("QWidget#mainWidget { background-image: url(gfx/background4.gif)}");
    setCentralWidget(mainWidget);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("About Juniper"));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exitAction);
    //These two don't look too good
    //menuBar()->setStyleSheet("QMenuBar { background-image: url(background1.gif)}");
    //fileMenu->setStyleSheet("QMenu { background-image: url(background1.gif)}");

    settingsMenu = menuBar()->addMenu(tr("&Settings"));
    formatMenu = settingsMenu->addMenu(tr("Rip format"));
    styleMenu = settingsMenu->addMenu(tr("Window style"));

    formatGroup = new QActionGroup(this);

    selectWavAction = new QAction(tr("&WAV"), formatGroup);
    selectFlacAction = new QAction(tr("&FLAC"), formatGroup);
    selectOggAction = new QAction(tr("&OGG"), formatGroup);
    selectMp3Action = new QAction(tr("&MP3"), formatGroup);
    selectWavAction->setCheckable(true);
    selectFlacAction->setCheckable(true);
    selectOggAction->setCheckable(true);
    selectMp3Action->setCheckable(true);

    selectMp3Action->setChecked(true);
    formatMenu->addAction(selectWavAction);
    formatMenu->addAction(selectFlacAction);
    formatMenu->addAction(selectOggAction);
    formatMenu->addAction(selectMp3Action);

    toggleAutoRipAction = new QAction(tr("&AutoRip"), this);
    toggleAutoRipAction->setCheckable(true);
    toggleAutoRipAction->setChecked(this->autoRip);

    settingsMenu->addAction(toggleAutoRipAction);
    settingsMenu->addMenu(formatMenu);
    settingsMenu->addMenu(styleMenu);

    styleActionGroup = new QActionGroup(this);
    foreach(QString styleName, QStyleFactory::keys()){
	QAction *action = new QAction(styleActionGroup);
	action->setText(tr("%1 Style").arg(styleName));
	action->setData(styleName);
	action->setCheckable(true);
	connect(action, SIGNAL(triggered(bool)), this, SLOT(changeStyle(bool)));
	styleMenu->addAction(action);
    }
    //checkCurrentStyle();

    aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);

    searchBox = new QLineEdit;
    searchButton = new QPushButton(tr("Search"));

    listView = new QPlayListView(this);

    QHeaderView *header = listView->horizontalHeader();
    header->setStretchLastSection(true);
    listView->setHorizontalHeader(header);
    listView->horizontalHeader()->resizeSection(0, listView->horizontalHeader()->frameSize().width()/3);
    listView->horizontalHeader()->resizeSection(1, listView->horizontalHeader()->frameSize().width()/3);
    listView->horizontalHeader()->resizeSection(2, listView->horizontalHeader()->frameSize().width()/3);

    progressTimeLabel = new QLabel("00:00");    
    totalTimeLabel = new QLabel("00:00");

    seekSlider = new QSlider(Qt::Horizontal);
    seekSlider->setRange(0,1000);
    seekSlider->setValue(0);

    buttonPanel = new QWidget;
    prevButton = new QPushButton(QPixmap("gfx/skip_backward.png"), "");
    //playButton = new QPushButton(tr("Play"));
    playButton = new QPushButton(QPixmap("gfx/play.png"), "");
    nextButton = new QPushButton(QPixmap("gfx/skip_forward.png"), "");
    netButton = new QPushButton(QPixmap("gfx/net.png"), "");

    quitButton = new QPushButton(tr("Quit"));
    quitButton->setFont(QFont("Times", 18, QFont::Bold));

    listSplitter = new QSplitter(Qt::Horizontal, this);
    listListView = new QListListView(this);

    listSplitter->addWidget(listListView);
    listSplitter->addWidget(listView);
    listSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QGridLayout *buttonsLayout = new QGridLayout;
    buttonPanel->setLayout(buttonsLayout);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    //buttonsLayout->setSpacing(0);
    buttonsLayout->addWidget(prevButton, 0, 0);
    buttonsLayout->addWidget(playButton, 0, 1);
    buttonsLayout->addWidget(nextButton, 0, 2);
    buttonsLayout->addWidget(progressTimeLabel, 0, 3);
    buttonsLayout->addWidget(seekSlider, 0, 4);
    buttonsLayout->addWidget(totalTimeLabel, 0, 5);
    buttonsLayout->addWidget(netButton, 0, 6);
    buttonPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QGridLayout *layout = new QGridLayout;
    
    layout->addWidget(searchBox, 0, 0, 1, 3);
    layout->addWidget(searchButton, 0, 3, 1, 1);
    layout->addWidget(listSplitter, 1, 0, 1, 4);
    //layout->addWidget(filler, 2, 0, 1, 2);
    layout->addWidget(buttonPanel, 2, 0, 1, 4);

    listSplitter->setSizes(QList<int>() << 10 << 10000);

    mainWidget->setLayout(layout);
    statusBar()->showMessage(QString("Juniper ready to rock"));
    setMinimumSize(300, 200);
    resize(800, 480);

}

void MainWindow::executeSearch()
{
  printf("MainWindow: slot executeSearch() was signaled!\n");

  const QString sstr = searchBox->text();
  //int length = sstr.length();
  //const QByteArray qba = sstr.toUtf8();
  //int arrsz = qba.size();
  //const char *ss = qba.data();
  //printf("Length of string was %d\n", length);
  //printf("Contents of bytearray: %s\n", ss);
  spotWorker->performSearch(searchBox->text());
}

void MainWindow::songDoubleClicked(const QModelIndex &index)
{
    printf("Song doubleclicked... Trying to play it...\n");

    sp_track* track = listListModel->getTrack(index);

    printf("Loading player...\n");
    spotWorker->loadPlayer(track, autoRip, ripFormat);
    printf("Playing player...\n");
    this->playButton->setText("Stop"); //TODO pål
    spotWorker->playPlayer(true);
}

void MainWindow::playStop()
{
    spotWorker->playPlayer(!spotWorker->isPlaying());
}


//public slot
void MainWindow::searchComplete(sp_search *search)
{
    printf("MainWindow: signal searchComplete received!\n");
    
    int i;
    
    printf("Query          : %s\n", sp_search_query(search));
    printf("Did you mean   : %s\n", sp_search_did_you_mean(search));
    printf("Tracks in total: %d\n", sp_search_total_tracks(search));
    
    for (i = 0; i < sp_search_num_tracks(search) && i < 40; ++i){

	sp_track *track = sp_search_track(search, i);
	//Extract interesting information
	int duration = sp_track_duration(track);
	//sp_album *talbum = sp_track_album(track);
	//const char *albumTitle = sp_album_name(talbum);
	//int artistCount = sp_track_num_artists(track);
	//sp_artist *tartist = sp_track_artist(track, 0);
	//const char *artistName = sp_artist_name(tartist);

	//test... play first hit
	//if(i==0){
	//    worker->loadPlayer(track);
	//    worker->playPlayer(true);
	//}
	
	printf("  Track \"%s\" [%d:%02d] has %d artist(s), %d%% popularity\n",
	       sp_track_name(track),
	       duration / 60000,
	       (duration / 1000) / 60,
	       sp_track_num_artists(track),
	       sp_track_popularity(track));

	//int rowIndex = playList->rowCount();
	//playList->insertRow(rowIndex);
	
	//QTableWidgetItem *titleCell = new QTableWidgetItem(QString::number(sp_track_num_artists(track)), 0);
	
	/*
	QTableWidgetItem *titleCell = new QTableWidgetItem(artistName, 0);
	QTableWidgetItem *artistCell = new QTableWidgetItem(sp_track_name(track), 0);
	QTableWidgetItem *albumCell = new QTableWidgetItem(albumTitle, 0);
	*/
	
	//plmodel->insertRows(tablePointer->rowCount(), track);
	    
	//playList->setItem(rowIndex, 0, titleCell);
	//playList->setItem(rowIndex, 1, artistCell);
	//playList->setItem(rowIndex, 2, albumCell);
    }

    printf("mainwindow.cpp: Adding search to listlistmodel\n");
    if(!listListModel) //this is kinda dirty. need cleanup
	listListModel = new QListListModel(NULL);
    listListModel->addSearch(search);
    //TODO: free the previous model before allocating a new
    printf("mainwindow.cpp: Creating searchlistmodel\n");
    searchListModel = new QSearchListModel(search, listView);
    printf("mainwindow.cpp: Setting searchlistmodel\n");
    listView->setModel(searchListModel);

    printf("mainwindow.cpp: Executing messy code\n");
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

	this->progressTimeLabel->setText(progressText);
	this->totalTimeLabel->setText(totalText);

	this->seekSlider->setValue(sliderValue);
    }
    else {
	this->seekSlider->setValue(0);
    }

    if(spotWorker->isPlaying())
	this->playButton->setIcon(QPixmap("gfx/stop.png"));
    else
	this->playButton->setIcon(QPixmap("gfx/play.png"));
}


void MainWindow::saveUser(QString username, QString password)
{
    //TODO: save the user
    username = username;
    password = password;
    printf("Saving user\n");
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
    if(!listListModel)
	listListModel = new QListListModel(plc);
    listListView->setModel(listListModel);
}


void MainWindow::selectWav()
{
    toggleFormat(SoundSaver::WAV);
    printf("File output format changed to: Wav\n");
}
void MainWindow::selectFlac()
{
    toggleFormat(SoundSaver::FLAC);
    printf("File output format changed to: Flac\n");
}
void MainWindow::selectOgg()
{
    toggleFormat(SoundSaver::OGG);
    printf("File output format changed to: Ogg\n");
}
void MainWindow::selectMp3()
{
    toggleFormat(SoundSaver::MP3);
    printf("File output format changed to: Mp3\n");
}

void MainWindow::toggleFormat(SoundSaver::FileType format)
{
    selectWavAction->setChecked(false);
    selectFlacAction->setChecked(false);
    selectOggAction->setChecked(false);
    selectMp3Action->setChecked(false);
    switch(format){
    case SoundSaver::WAV:
	selectWavAction->setChecked(true);
	break;
    case SoundSaver::FLAC:
	selectFlacAction->setChecked(true);
	break;
    case SoundSaver::OGG:
	selectOggAction->setChecked(true);
	break;
    case SoundSaver::MP3:
	selectMp3Action->setChecked(true);
	break;
    }
    this->ripFormat = format;
}

void MainWindow::toggleAutoRip(bool rip)
{
    printf("AutoRip toggled\n");
    this->autoRip = rip;
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
    foreach (QAction *action, styleActionGroup->actions()) {
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
    printf("List in playlistlist clicked, index: %d\n", index.row());
    if(listListModel->isSearchList(index)){
	printf("Searchlist clicked\n");
	searchListModel = new QSearchListModel(listListModel->getSearchList(index), listView);
	printf("Setting model\n");
	this->listView->setModel(playListModel);
	printf("Model set\n");
    }
    else{
	printf("Playlist clicked\n");
	playListModel = new QPlayListModel(listListModel->getPlayList(index), listView);
	printf("Setting model\n");
	this->listView->setModel(playListModel);
	printf("Model set\n");
    }

    //connect(listView, SIGNAL(doubleClicked(const QModelIndex)),
    //	this, SLOT(songDoubleClicked(const QModelIndex)) );
    
}

void MainWindow::listListDoubleClicked(const QModelIndex &/*item*/)
{
    //TODO: pål
    printf("Playlist clicked...\n");
}

QString MainWindow::getUsername(void)
{
  QFile file("user.cfg");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
    printf("Failed to open user file\n");
    return NULL;
  }
  
  QTextStream in(&file);
  QString line = in.readLine();

  const QByteArray qba = line.toUtf8();
  const char *ss = qba.data();

  printf("Returning username: %s\n", ss);

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

  printf("Returning password: %s\n", ss);
  
  return line;
}
