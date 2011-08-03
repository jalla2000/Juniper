/**
 * @file settingsdialog.cpp
 * @author Bernd Wachter <bwachter@lart.info>
 * @date 2011
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

#include "settingsdialog.h"

SettingsDialog::SettingsDialog(): QDialog(){
    setupUi(this);

    bool initialized=settings.value("initialized").toBool();
    if (!initialized){
        qDebug() << "Setting initial settings...";
        settings.setValue("initialized", true);
//        settings.setValue("ripFormat", SoundSaver::MP3);
        settings.setValue("autoRip", false);
        settings.setValue("initialX", 800);
        settings.setValue("initialY", 480);
        settings.setValue("spotify/cache", QDir::homePath() + "/.cache/juniper/");
        settings.setValue("spotify/config", QDir::homePath() + "/.config/Juniper/");
    }

    mainInitialized->setChecked(!settings.value("initialized").toBool());
    mainFocusFollowsMouse->setChecked(settings.value("focusFollowsMouse").toBool());
    mainShowStatusbar->setChecked(settings.value("showStatusbar").toBool());
    mainInitialX->setValue(settings.value("initialX").toInt());
    mainInitialY->setValue(settings.value("initialY").toInt());
    mainSaveSizeOnExit->setChecked(settings.value("saveSizeOnExit").toBool());

    settings.beginGroup("spotify");
    spotifyUsername->setText(settings.value("username").toString());
    spotifyPassword->setText(settings.value("password").toString());
    spotifyCache->setText(settings.value("cache").toString());
    spotifyConfig->setText(settings.value("config").toString());
    settings.endGroup();

    settings.beginGroup("player");
    playerLoadAction->setCurrentIndex(settings.value("loadAction").toInt());
    settings.endGroup();
}

void SettingsDialog::accept(){
    qDebug() << "Saved settings";

    settings.setValue("initialized", !mainInitialized->isChecked());
    settings.setValue("focusFollowsMouse", mainFocusFollowsMouse->isChecked());
    settings.setValue("showStatusbar", mainShowStatusbar->isChecked());
    settings.setValue("initialX", mainInitialX->value());
    settings.setValue("initialY", mainInitialY->value());
    settings.setValue("saveSizeOnExit", mainSaveSizeOnExit->isChecked());

    settings.beginGroup("spotify");
    settings.setValue("username", spotifyUsername->text());
    settings.setValue("password", spotifyPassword->text());
    settings.setValue("cache", spotifyCache->text());
    settings.setValue("config", spotifyConfig->text());
    settings.endGroup();

    settings.beginGroup("player");
    settings.setValue("loadAction", playerLoadAction->currentIndex());
    settings.endGroup();

    this->hide();
    emit configurationChanged();
}
