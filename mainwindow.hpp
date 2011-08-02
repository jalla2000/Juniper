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

#include "ui_juniper.h"

#include "spotworker.hpp"
#include "qlistlistmodel.hpp"
#include "tracklistmodel.h"
#include "settingsdialog.h"

class MainWindow : public QMainWindow, private Ui::Juniper {

    Q_OBJECT

 public:
    MainWindow(QWidget *parent = 0);
    SpotWorker *spotWorker; //TODO: make private!

 private:
    QTimer *guiUpdater_;

    QSettings settings;
    SettingsDialog *settingsDialog;

    //models and structure
    QListListModel *listListModel_;
    TrackListModel *trackListModel;

    SoundSaver::FileType ripFormat_;
    QActionGroup *formatGroup_;

    //functions
    void connectSignals();

 private slots:
    void toggleRipFormat();

 public slots:
    void songDoubleClicked(const QModelIndex &index);
    void listListClicked(const QModelIndex &index);
    void loginFailed(void);
    void loggedIn();
    void about();
    void updateGui();
    void toggleAutoRip(bool);
    void playStop();
    void showSettings();

 protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);

};

#endif
