/**
 * @file soundsaver.cpp
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

#include <stdlib.h>
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <sndfile.h>
#include "soundsaver.hpp"


SoundSaver::SoundSaver(QObject *parent)
    : QObject(parent)
{
    writing_ = false;
    fileHandle_ = NULL;
    //TODO: this might not always be the case?
    wformat_.samplerate = 44100;
    wformat_.channels = 2;
    writeMutex_ = new QMutex();
}

void SoundSaver::open(const char *path, FileType type)
{
    writeMutex_->lock();
    if(writing_){
        writeMutex_->unlock();
        close();
        writeMutex_->lock();
    }

    fileName_ = new QString(QString().fromUtf8(path));
    fileType_ = type;

    switch(type){
    case OGG:
        wformat_.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS; //SF_FORMAT_PCM_16;
        fileExtension_ = new QString(".ogg");
        break;
    case FLAC:
        wformat_.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
        fileExtension_ = new QString(".fla");
        break;
    case WAV:
        wformat_.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        fileExtension_ = new QString(".wav");
        break;
    case MP3:
        wformat_.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        fileExtension_ = new QString(".wav");
        break;
    default:
        printf("Unknown file format. Defaulting to WAV.\n");
        wformat_.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        fileExtension_ = new QString(".wav");
        break;
    }

    printf("Opening new file: %s%s\n",
           fileName_->toUtf8().data(),
           fileExtension_->toUtf8().data());

    fileHandle_ = sf_open(getCurrentFileName().toUtf8().data(), SFM_WRITE, &wformat_);
    if(!fileHandle_)
        printf("Failed to open output file %s\n", path);

    writing_ = true;
    writeMutex_->unlock();

}

void SoundSaver::saveSound(const void *frames, int count)
{
    static bool errorReported = false;
    writeMutex_->lock();

    if(writing_){
        const int16_t *samples = reinterpret_cast<const int16_t *>(frames);
        int written = sf_writef_short(fileHandle_, samples, count);
        if(written!=count && !errorReported){
            printf("Error! trouble!\n");
            errorReported = true;
        }
        else {
            errorReported = false;
        }
    }

    writeMutex_->unlock();
}

void SoundSaver::close()
{
    writeMutex_->lock();
    if(writing_){
        //printf("Syncing and cloing file\n");
        sf_write_sync(fileHandle_);
        sf_close(fileHandle_);
        //      QByteArray qba = this->currentFileName.toUtf8()
        printf("Closed file: %s\n", fileName_->toUtf8().data());
        if(fileType_==MP3){
            printf("Converting to MP3\n");
            QString lameCommand("");
            lameCommand += QString("(lame -b 192 -h -V 4 \"") +
                QString(fileName_->toUtf8()) +
                QString(fileExtension_->toUtf8()) +
                QString("\" \"") +
                QString(fileName_->toUtf8()) +
                QString(".mp3") +
                QString("\") &");
            printf("Lamecommand: %s\n", lameCommand.toUtf8().data());
            // TODO: it is not recommended to discard the return value of system().
            // it should always be evaluated, or there will be no cake
            // (https://wiki.ubuntu.com/CompilerFlags)
            if(system(lameCommand.toUtf8().data())){};
        }
        writing_ = false;
    }
    writeMutex_->unlock();
}

QString SoundSaver::getCurrentFileName()
{
    return QString(QString(fileName_->toUtf8()) + QString(fileExtension_->toUtf8()));
}
