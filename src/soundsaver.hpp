/**
 * @file soundsaver.hpp
 * @author Pål Driveklepp <jalla2000@gmail.com>
 * @date 2009
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

#ifndef SOUNDSAVER_H
#define SOUNDSAVER_H

#include <QObject>
#include <QMutex>
#include <sndfile.h>


class SoundSaver : public QObject {

    Q_OBJECT

 public:
    enum FileType { WAV, FLAC, OGG, MP3 };
    SoundSaver(QObject *parent = 0);
    void open(const char *path, FileType type);
    void saveSound(const void *frames, int count);
    void close();
    QString getCurrentFileName();
 private:
    volatile bool writing_;
    QMutex *writeMutex_;
    FileType fileType_;
    int totalFrames_;
    int frameCounter_;
    SF_INFO wformat_;
    SNDFILE *fileHandle_;
    QString *fileName_;
    QString *fileExtension_;
 signals:

 public slots:

};


#endif
