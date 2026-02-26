/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <QDebug>
#include "core/MediaManager.h"



MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<AudioFrame>("AudioFrame");

       m_timer = new QTimer(this);

       connect(m_timer, &QTimer::timeout, this, [this]() {

           if (!m_stream) return;

           DWORD state = BASS_ChannelIsActive(m_stream);

           if (state == BASS_ACTIVE_PLAYING)
           {
               AudioFrame frame;

               frame.position = BASS_ChannelBytes2Seconds(
                   m_stream,
                   BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE)
               );

               DWORD level = BASS_ChannelGetLevel(m_stream);

               frame.left  = LOWORD(level) / 32768.0f;
               frame.right = HIWORD(level) / 32768.0f;

               emit audioFrameUpdated(frame);
           }
           else if (state == BASS_ACTIVE_STOPPED)
           {
               m_timer->stop();

               AudioFrame frame;
               frame.position = 0.0;
               frame.left = 0.0f;
               frame.right = 0.0f;

               emit audioFrameUpdated(frame);
               emit playbackFinished();
           }
       });

       //m_timer->start(50); // 20 FPS
}


MediaManager::~MediaManager(){}



bool MediaManager::loadFile(const QString &filePath){

    if (m_stream) {
        BASS_StreamFree(m_stream);
        m_stream = 0;
    }

#ifdef Q_OS_WIN
    m_stream = BASS_StreamCreateFile(
        FALSE,
        filePath.utf16(),
        0,
        0,
        BASS_UNICODE
    );
#else
    QByteArray path = filePath.toUtf8();
    m_stream = BASS_StreamCreateFile(
        FALSE,
        path.constData(),
        0,
        0,
        0
    );
#endif

    if (!m_stream) {
         return false;
       }


       BASS_ChannelSetSync(
           m_stream,
           BASS_SYNC_END,
           0,
           &MediaManager::EndSyncCallback,
           this
       );

       return true;
}

//******************************************************************


double MediaManager::getDurationSecond(const QString &filePath){

#ifdef Q_OS_WIN
    HSTREAM stream = BASS_StreamCreateFile(
        FALSE,
        filePath.utf16(),
        0,
        0,
        BASS_STREAM_DECODE | BASS_UNICODE
    );
#else
    QByteArray path = filePath.toUtf8();

    HSTREAM stream = BASS_StreamCreateFile(
        FALSE,
        path.constData(),
        0,
        0,
        BASS_STREAM_DECODE
    );
#endif

    if (!stream) {
        qDebug() << "BASS error:" << BASS_ErrorGetCode()
                 << "File:" << filePath;
        return -1.0;
    }

    QWORD length = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
    double seconds = BASS_ChannelBytes2Seconds(stream, length);

    BASS_StreamFree(stream);
    return seconds;
}



//****************************************************


void MediaManager::play()
{
    if (!m_stream) return;

        BASS_ChannelPlay(m_stream, FALSE);
        m_timer->start(50);
}

void MediaManager::pause()
{
    if (!m_stream) return;

        BASS_ChannelPause(m_stream);
        m_timer->stop();
}

void MediaManager::stop()
{
    if (!m_stream) return;

        BASS_ChannelStop(m_stream);
        m_timer->stop();

        emit positionChanged(0.0);
}


bool MediaManager::isPlaying() const
{
    if (!m_stream) return false;
    return BASS_ChannelIsActive(m_stream) == BASS_ACTIVE_PLAYING;
}

bool MediaManager::isPaused() const
{
    if (!m_stream) return false;
    return BASS_ChannelIsActive(m_stream) == BASS_ACTIVE_PAUSED;
}

void MediaManager::rewind()
{
    seekRelative(-5.0);
}

void MediaManager::forward()
{
    seekRelative(5.0);
}

void MediaManager::seek(double seconds)
{
    if (!m_stream) return;

    // Duración total
    double duration = BASS_ChannelBytes2Seconds(
        m_stream,
        BASS_ChannelGetLength(m_stream, BASS_POS_BYTE)
    );


    if (seconds < 0.0)
        seconds = 0.0;

    if (seconds > duration)
        seconds = duration;

    // Convertir y aplicar
    QWORD bytePos = BASS_ChannelSeconds2Bytes(m_stream, seconds);
    BASS_ChannelSetPosition(m_stream, bytePos, BASS_POS_BYTE);

    // 🔥 Actualizar UI inmediatamente (sin esperar timer)
    DWORD level = BASS_ChannelGetLevel(m_stream);

    AudioFrame frame;
    frame.position = seconds;
    frame.left  = LOWORD(level) / 32768.0f;
    frame.right = HIWORD(level) / 32768.0f;

    emit audioFrameUpdated(frame);
}




void MediaManager::seekRelative(double deltaSeconds)
{
    if (!m_stream) return;

      double current = BASS_ChannelBytes2Seconds(
          m_stream,
          BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE)
      );

      seek(current + deltaSeconds);
}



//****************************************************



void CALLBACK MediaManager::EndSyncCallback(HSYNC, DWORD, DWORD, void *user)
{
    MediaManager* self = static_cast<MediaManager*>(user);

    if (!self) return;

    QMetaObject::invokeMethod(
        self,
        [self]()
        {
            if (self->m_timer)
                self->m_timer->stop();

            emit self->playbackFinished();
        },
        Qt::QueuedConnection
    );
}

