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

#include <stdlib.h>
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <sndfile.h>
#include "soundsaver.hpp"


SoundSaver::SoundSaver(QObject *parent)
    : QObject(parent)
{
    writing = false;
    fileHandle = NULL;
    //TODO: this might not always be the case?
    wformat.samplerate = 44100;
    wformat.channels = 2;
    writeMutex = new QMutex();
}

void SoundSaver::open(const char *path, FileType type)
{
    writeMutex->lock();
    if(writing){
	writeMutex->unlock();
	close();
	writeMutex->lock();
    }

    fileName = new QString(QString().fromUtf8(path));
    fileType = type;

    switch(type){
    case OGG:
	wformat.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS; //SF_FORMAT_PCM_16;
	fileExtension = new QString(".ogg");
	break;
    case FLAC:
	wformat.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
	fileExtension = new QString(".fla");
	break;
    case WAV:
	wformat.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	fileExtension = new QString(".wav");
	break;
    case MP3:
	wformat.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	fileExtension = new QString(".wav");
	break;
    default:
	printf("Unknown file format. Defaulting to WAV.\n");
	wformat.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	fileExtension = new QString(".wav");
	break;
    }

    printf("Opening new file: %s%s\n", 
	   this->fileName->toUtf8().data(),
	   this->fileExtension->toUtf8().data());

    fileHandle = sf_open(getCurrentFileName().toUtf8().data(), SFM_WRITE, &wformat);
    if(!fileHandle)
	printf("Failed to open output file %s\n", path);

    writing = true;
    writeMutex->unlock();

}

void SoundSaver::saveSound(const void *frames, int count)
{
    static bool errorReported = false;
    writeMutex->lock();
    
    if(writing){
	const int16_t *samples = reinterpret_cast<const int16_t *>(frames);
	int written = sf_writef_short(fileHandle, samples, count);
	if(written!=count && !errorReported){
	    printf("Error! trouble!\n");
	    errorReported = true;
	}
	else {
	    errorReported = false;
	}
    }

    writeMutex->unlock();
}

void SoundSaver::close()
{
    writeMutex->lock();
    if(writing){
	//printf("Syncing and cloing file\n");
	sf_write_sync(fileHandle);
	sf_close(fileHandle);
	//	QByteArray qba = this->currentFileName.toUtf8()
	printf("Closed file: %s\n", this->fileName->toUtf8().data());
	if(fileType==MP3){
	    printf("Converting to MP3\n");
	    QString lameCommand("");
	    lameCommand += QString("(lame -b 192 -h -V 4 \"") + 
		QString(this->fileName->toUtf8()) +
		QString(this->fileExtension->toUtf8()) +
		QString("\" \"") + 
		QString(this->fileName->toUtf8()) + 
		QString(".mp3") +
		QString("\") &");
	    printf("Lamecommand: %s\n", lameCommand.toUtf8().data());
	    // TODO: it is not recommended to discard the return value of system().
	    // it should always be evaluated, or there will be no cake 
	    // (https://wiki.ubuntu.com/CompilerFlags)
	    if(system(lameCommand.toUtf8().data())){};
	}
	writing = false;
    }
    writeMutex->unlock();
}

QString SoundSaver::getCurrentFileName()
{
    return QString(QString(fileName->toUtf8()) + QString(fileExtension->toUtf8()));
}
