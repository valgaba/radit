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
#include <QCoreApplication>
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

               float leftLinear  = LOWORD(level) / 32768.0f;
               float rightLinear = HIWORD(level) / 32768.0f;

               leftLinear  = std::max(leftLinear,  0.000001f);
               rightLinear = std::max(rightLinear, 0.000001f);

               frame.left  = 20.0f * log10f(leftLinear);
               frame.right = 20.0f * log10f(rightLinear);

               //  detección de finales
                      if (shouldStopBySilence(frame))
                      {
                          BASS_ChannelStop(m_stream);
                          m_timer->stop();

                          emit playbackFinished();
                          return;
                      }



               emit audioFrameUpdated(frame);
           }

       });

       //m_timer->start(50); // 20 FPS
}


MediaManager::~MediaManager(){

    if (m_stream){
        BASS_StreamFree(m_stream);
        m_stream = 0;
       }


}


//*****************************************

bool MediaManager::initialize()
{
    int device = -1; // default Windows

    BASS_SetConfig(BASS_CONFIG_BUFFER, 5000);
    BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);

    if (!BASS_Init(device, 44100, 0, nullptr, nullptr))
    {
        qDebug() << "BASS_Init error:" << BASS_ErrorGetCode();
        return false;
    }

#ifdef Q_OS_WIN

    QString basePath = QCoreApplication::applicationDirPath() + "/bassplugin";

    QStringList plugins = {
        "bass_aac.dll",
        "bassflac.dll",
        "basswma.dll"
    };

    for (const QString &plugin : plugins)
    {
        QString fullPath = basePath + "/" + plugin;

        HPLUGIN handle = BASS_PluginLoad(
            fullPath.toUtf8().constData(), 0
        );

        if (!handle){

            qDebug() << "Error cargando plugin:"
                     << fullPath
                     << "Error code:"
                     << BASS_ErrorGetCode();

        }
    }

#endif

#ifdef Q_OS_UNIX
    QString basePath = QCoreApplication::applicationDirPath() + "/Plugin";
    BASS_PluginLoad((basePath + "/libbass_aac.so").toUtf8(), 0);
    BASS_PluginLoad((basePath + "/libbassflac.so").toUtf8(), 0);
#endif

    return true;
}


void MediaManager::shutdown(){

    if (m_stream)
    {
        BASS_StreamFree(m_stream);
        m_stream = 0;
    }

    BASS_Free();
}



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
    if (!m_stream)
         return;


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
    seekRelative(-1.0); // esta a 1 segundo
}

void MediaManager::forward()
{
    seekRelative(1.0);
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

       //  Actualizar UI inmediatamente
       DWORD level = BASS_ChannelGetLevel(m_stream);

       AudioFrame frame;

       frame.position = seconds;

       float leftLinear  = LOWORD(level) / 32768.0f;
       float rightLinear = HIWORD(level) / 32768.0f;

       leftLinear  = std::max(leftLinear,  0.000001f);
       rightLinear = std::max(rightLinear, 0.000001f);

       frame.left  = 20.0f * log10f(leftLinear);
       frame.right = 20.0f * log10f(rightLinear);

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


//**************************
bool MediaManager::setDevice(int deviceId){

    BASS_DEVICEINFO info;
    BASS_GetDeviceInfo(deviceId, &info);


     // Solo inicializar si NO está inicializado
         if (!(info.flags & BASS_DEVICE_INIT)){

             if (!BASS_Init(deviceId, 44100, 0, nullptr, nullptr)){
                 qDebug() << "Error initializing device:" << BASS_ErrorGetCode();
                 return false;
             }
         }

         // SIEMPRE cambiar al dispositivo
             if (!BASS_SetDevice(deviceId)){
                qDebug() << "Error setting device:" << BASS_ErrorGetCode();
                 return false;
             }



    return true;
}


//******************************
void MediaManager::fadeOut(int durationMs)
{
    if (!m_stream)
        return;

    // Deslizar volumen hasta 0
    BASS_ChannelSlideAttribute(
        m_stream,
        BASS_ATTRIB_VOL,
        0.0f,
        durationMs
    );

    // Sync cuando termina el slide
    BASS_ChannelSetSync(
        m_stream,
        BASS_SYNC_SLIDE,
        0,
        &MediaManager::FadeOutSyncCallback,
        this
    );
}




//****************************************************

void CALLBACK MediaManager::EndSyncCallback(
        HSYNC,
        DWORD,
        DWORD,
        void *user)
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


//*****************************************
void CALLBACK MediaManager::FadeOutSyncCallback(
        HSYNC,
        DWORD channel,
        DWORD,
        void *user)
        {
            MediaManager* self = static_cast<MediaManager*>(user);
            if (!self) return;

            QMetaObject::invokeMethod(
                self,
                [self, channel]()
                {
                    // Restaurar volumen
                    BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 1.0f);

                    // Stop
                    BASS_ChannelStop(channel);

                    // Posición al inicio
                    BASS_ChannelSetPosition(channel, 0, BASS_POS_BYTE);

                    // Si es el canal actual, mantener coherencia
                    if (channel == self->m_stream)
                    {
                        if (self->m_timer)
                            self->m_timer->stop();

                        emit self->playbackFinished();
                    }
                },
                Qt::QueuedConnection
            );
}

bool MediaManager::shouldStopBySilence(const AudioFrame& frame)
{
    float maxDb = std::max(frame.left, frame.right);

    // 1. Detectar si hubo audio real
    if (maxDb > m_soundThresholdDb)
    {
        m_hadSound = true;
    }

    // 2. Duración total
    double duration = BASS_ChannelBytes2Seconds(
        m_stream,
        BASS_ChannelGetLength(m_stream, BASS_POS_BYTE)
    );

    double current = frame.position;

    // margen dinámico
    double tailMargin = std::max(m_tailSeconds, duration * m_tailPercent);

    bool nearEnd = (duration - current) < tailMargin;

    // 3. Contador de silencio
    if (maxDb < m_silenceThresholdDb)
    {
        m_silenceCounter += 50;
    }
    else
    {
        m_silenceCounter = 0;
    }


    // 4. Decisión final
    return (nearEnd &&
            m_hadSound &&
            m_silenceCounter >= m_silenceDurationMs);
}
