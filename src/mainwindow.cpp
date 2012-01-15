/**
 * @file mainwindow.cpp
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

#include <QStyleFactory>
#include <QMessageBox>

#include "mainwindow.hpp"
#include "spotworker.hpp"
#include "tracklistmodel.hpp"

#define DEBUGLEVEL 1
#define DEBUG if(DEBUGLEVEL)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
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

    settingsDialog_ = new SettingsDialog;

    if (settings_.value("spotify/username").toString().isEmpty())
        settingsDialog_->show();

    searchBox->insertItems(0, settings_.value("savedSearch").toStringList());

    listListModel_ = new QListListModel();
    listListView->setModel(listListModel_);

    trackListModel_ = new TrackListModel();
    trackList->setModel(trackListModel_);

    ripFormat_ = static_cast<SoundSaver::FileType>(settings_.value("ripFormat").toUInt());

    guiUpdater_ = new QTimer;
    spotWorker = SpotWorker::getInstance();
    if (!spotWorker->start(settings_.value("spotify/username").toString(),
                          settings_.value("spotify/password").toString()))
    {
        printf("Failet to start SpotWorker!\n");
        assert(false);
    }
    qDebug() << "SpotWorker started";

    connectSignals();
    qDebug() << "Signals connected";

    guiUpdater_->start(200);

    this->show();

    statusBar()->showMessage(QString("Juniper ready to rock"));
    resize(settings_.value("initialX").toInt(),
           settings_.value("initialY").toInt());

}

void MainWindow::connectSignals()
{
    connect(guiUpdater_, SIGNAL(timeout()),
            this, SLOT(updateGui()) );

    connect(spotWorker, SIGNAL(playlistAdded(sp_playlistcontainer*)),
            listListModel_, SLOT(setPlayLists(sp_playlistcontainer*)) );

    connect(searchBox, SIGNAL(currentIndexChanged(QString)),
            spotWorker, SLOT(performSearch(QString)));

    //connect(seekSlider, SIGNAL(sliderMoved(int)),
    //        spotWorker, SLOT(seekPlayer(int)));
    connect(netButton, SIGNAL(clicked()), spotWorker, SLOT(startServer()));

    connect(listListView, SIGNAL(clicked(const QModelIndex)),
            this, SLOT(listListClicked(const QModelIndex)) );

    connect(spotWorker, SIGNAL(searchComplete(sp_search*)),
            trackListModel_, SLOT(setSearch(sp_search*)));

    connect(spotWorker, SIGNAL(loggedOut(sp_session*)),
            this, SLOT(loginFailed()) );
    connect(spotWorker, SIGNAL(loggedIn(sp_session*, sp_error*)),
            this, SLOT(loggedIn()) );

    connect(trackList, SIGNAL(doubleClicked(const QModelIndex)),
            this, SLOT(songDoubleClicked(const QModelIndex)) );
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addMenu(fileMenu);
    menu.addMenu(settingsMenu);
    menu.addMenu(helpMenu);
    menu.exec(event->globalPos());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QStringList searches;
    for (int i=0; i<20&&i<searchBox->count(); i++){
        searches.append(searchBox->itemText(i));
    }
    settings_.setValue("savedSearch", searches);

    event->accept();
}

void MainWindow::showSettings()
{
    settingsDialog_->show();
}

void MainWindow::songDoubleClicked(const QModelIndex &index)
{
    qDebug() << "Song doubleclicked... Trying to play it...";

    sp_track* track = trackListModel_->getTrack(index);

    qDebug() << "Loading player...";
    spotWorker->loadPlayer(track, settings_.value("autoRip").toBool(), ripFormat_);
    qDebug() << "Playing player...";
    spotWorker->playPlayer(true);
}

void MainWindow::playStop()
{
    spotWorker->playPlayer(!spotWorker->isPlaying());
}


void MainWindow::loggedIn()
{
    searchBox->setEnabled(true);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Juniper"),
                       tr("<b>Juniper</b> is the most epic spotify client ever "
                          "<br>Creds to my peeps"));
}

void MainWindow::loginFailed()
{
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
    settings_.setValue("ripFormat", format);
}

void MainWindow::toggleAutoRip(bool rip)
{
    qDebug() << "AutoRip toggled";
    settings_.setValue("autoRip", rip);
}

void MainWindow::listListClicked(const QModelIndex & index)
{
    //TODO: pål
    DEBUG printf("List in playlistlist clicked, row/col=%d/%d\n",
                 index.row(), index.column());
    if(listListModel_->isSearchList(index)){
        qDebug() << "Searchlist clicked...";
        trackListModel_->setSearch(listListModel_->getSearchList(index));
    }
    else{
        DEBUG printf("MainWindow::%s: fetching playlist at col/row=%d/%d\n",
                     __func__, index.column(), index.row());
        assert(trackListModel_);
        assert(listListModel_);
        qDebug() << "Playlist clicked...";
        sp_playlist * selectedPlayList = listListModel_->getPlayList(index);
        trackListModel_->setPlaylist(selectedPlayList);
    }
}
