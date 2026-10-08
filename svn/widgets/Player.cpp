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
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include "widgets/Player.h"
#include "widgets/vumeter.h"
#include "widgets/contentsbase.h"
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/AudioItemMeteoClockMaxi.h"
#include <cmath>
#include <algorithm>

namespace {
struct MixGains { float incoming; float outgoing; };

MixGains equalPowerGains(float progress)
{
    const float angle=std::clamp(progress,0.0f,1.0f)*1.57079632679f;
    return {std::sin(angle),std::cos(angle)};
}

AudioItemMaxi *folderTransitionTarget(AudioItemFolderMaxi *folder)
{
    if (folder->isLoop()) return folder;
    QWidget *owner = folder->parentWidget();
    while (owner && !qobject_cast<ContentsBase*>(owner)) owner = owner->parentWidget();
    auto *contents = qobject_cast<ContentsBase*>(owner);
    if (!contents || !contents->layout) return nullptr;
    const int index = contents->layout->indexOf(folder);
    if (index < 0) return nullptr;
    for (int offset = 1; offset < contents->layout->count(); ++offset) {
        const int row = (index + offset) % contents->layout->count();
        auto *item = qobject_cast<AudioItemMaxi*>(contents->layout->itemAt(row)->widget());
        if (item && item->isPlayNext()) return item;
    }
    return nullptr;
}
}

//#include "widgets/container.h"


Player::Player(QWidget *parent) : Frame(parent) {
    setAcceptDrops(true);



    frameoptionsplayer = new FrameOptionsPlayer(this);



       mediamanager = new MediaManager(this);


    bindMediaManager();
    m_mixTimer = new QTimer(this);
    m_mixTimer->setInterval(20);
    m_mixTimer->setTimerType(Qt::PreciseTimer);
    connect(m_mixTimer, &QTimer::timeout, this, &Player::updateMix);
    m_playbackLimitTimer=new QTimer(this);
    m_playbackLimitTimer->setSingleShot(true);
    connect(m_playbackLimitTimer,&QTimer::timeout,this,[this]() {
        m_playbackLimitRemainingMs=0;
        if (currentItem && currentItem->playbackLimitSeconds()>0.0)
            mediamanager->finishPlayback();
    });

      layout = new QVBoxLayout(this);  // layout general
      layout->setContentsMargins(0, 0, 0, 0);
      layout->setSpacing(0);

      framebarra = new Frame(this);
      frametop = new Frame(this);
      framecenter = new Frame(this);
      framedown = new Frame(this);

      framecenter->setObjectName("framecenter");
      frametop->setObjectName("frametop");
      framedown->setObjectName("framedown");


      layoutbarra = new QHBoxLayout(framebarra);
      layouttop = new QHBoxLayout(frametop);
      layoutcenter = new QHBoxLayout(framecenter);
      layoutdown = new QHBoxLayout(framedown);
    //  layouttab = new QVBoxLayout(frametab);

      // quitar márgenes internos también
      layoutbarra->setContentsMargins(0, 0, 0, 0);
      layouttop->setContentsMargins(0, 0, 0, 0);
      layoutcenter->setContentsMargins(0, 0, 0, 0);
      layoutdown->setContentsMargins(0, 0, 0, 0);
     // layouttab->setContentsMargins(0, 0, 0, 0);

      layoutbarra->setSpacing(0);
      layouttop->setSpacing(5);
      layoutcenter->setSpacing(0);
      layoutdown->setSpacing(5);
     // layouttab->setSpacing(0);


      layout->addWidget(framebarra);
      layout->addWidget(frametop);
      layout->addWidget(framecenter);
      layout->addWidget(framedown);
      //layout->addWidget(frametab);



      // partes fijas
      framebarra->setFixedHeight(25);
      frametop->setFixedHeight(30);
      framecenter->setFixedHeight(45);
      framedown->setFixedHeight(30);

      // parte flexible
      layout->setStretch(4, 1); // frametab

      // pintar frametop de azul
      framebarra->setStyleSheet("background-color: #4e4d7a;");
    // this->setStyleSheet("background-color: #343434;");




      labeltitle =new Label(this);
      labeltitle->setText("Player [noname]");
      labeltitle->setStyleSheet("font-size: 14px;");
      btnclose = new Button(this);
      btnclose->setStyleSheet(
          "QPushButton {"
          "   border: none;"
          "   background: transparent;"
          "   padding: 0px;"
          "}"
      );


      btnclose->setFixedSize(15, 15);
      btnclose->setCursor(Qt::PointingHandCursor);
      btnclose->SetIcon("Close-hover.svg");
      btnclose->setToolTip("Close player");

      layoutbarra->addWidget(labeltitle);
      layoutbarra->addStretch();
      layoutbarra->addWidget(btnclose);

      connect(btnclose, &QPushButton::clicked, this, [this]() {
                        this->hide();

       });


     // Parte alta********************************

      vumeter = new VuMeter(frametop);
      vumeter->setFixedWidth(180);
      layouttop->addWidget(vumeter);
      layouttop->addStretch(1);
      btnoption= new Button(this);
      btnoption->SetIcon("Tools.svg");
      btnoption->setIconSize(QSize(20, 20));
      btnoption->setFixedSize(23, 23);  //Tamaño fijo
      btnoption->setToolTip("options.");

      layouttop->addWidget(btnoption);

      connect(btnoption, &QPushButton::clicked,
              this, [this](){

        //  if (!frameoptionsplayer)
        //  {
             /* frameoptionsplayer = new FrameOptionsPlayer(nullptr);

              frameoptionsplayer->setWindowFlags(
                  Qt::Popup | Qt::FramelessWindowHint
              );

              frameoptionsplayer->setFixedSize(300, 150);*/
         // }

          QPoint posGlobal = btnoption->mapToGlobal(
              QPoint(0, btnoption->height())
          );

          QScreen *screen = QGuiApplication::screenAt(posGlobal);

          if (!screen)
              screen = QGuiApplication::primaryScreen();

          QRect area = screen->availableGeometry();

          int x = posGlobal.x();
          int y = posGlobal.y();

          // Evitar que salga por la derecha
          if (x + frameoptionsplayer->width() > area.right() + 1)
          {
              x = area.right() + 1
                  - frameoptionsplayer->width();
          }

          // Evitar que salga por la izquierda
          if (x < area.left())
              x = area.left();

          // Si no cabe debajo, colocarlo encima
          if (y + frameoptionsplayer->height() > area.bottom() + 1)
          {
              y = btnoption->mapToGlobal(
                  QPoint(0, -frameoptionsplayer->height())
              ).y();
          }

          // Evitar que salga por arriba
          if (y < area.top())
              y = area.top();
          //frameoptionsplayer->UpdateDevice();
          frameoptionsplayer->move(x, y);
          frameoptionsplayer->show();
          frameoptionsplayer->raise();
      });




    //parte central
      btnstop = new Button(this);
      btnstop->setObjectName("btnstop");
      btnstop->SetIcon("eject.svg");
      btnstop->setIconSize(QSize(30, 40));  // ajusta al tamaño que quieras
      btnstop->setFixedSize(50, 30);  //Tamaño fijo
      btnstop->setToolTip("Eject");

      labelnombre = new Label;
      labelnombre->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      labeltiempo = new Label;
      labeltiempo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      labeltiempo->setFixedWidth(125);   // Fija solo el ancho
      labelnombre->setObjectName("labelnombre");
      labeltiempo->setObjectName("labeltiempo");

      // Cambiar tamaño de fuente
      QFont font = labeltiempo->font();
      font.setPointSize(16);
      font.setBold(true);
      labeltiempo->setFont(font);
      labeltiempo->setText("00:00:00.00");

      labeltiempo->setFixedHeight(25); //45
      labelnombre->setFixedHeight(45);




      layoutcenter->addWidget(btnstop);
      layoutcenter->addWidget(labelnombre);
      layoutcenter->addWidget(labeltiempo);

      layoutcenter->addStretch(); // todo a la izquierda

      connect(btnstop, &QPushButton::clicked, this, &Player::stopMain);



      ///***************** botonera parte baja ****************
              btnpause = new Button;
              btnpause->SetIcon("Pausemini.svg");
            //  btnpause->setIconSize(QSize(35, 35));   // no termina de gustarme el tamaño del icono por defecto
              btnpause->setIconSize(QSize(25, 25));
              btnpause->setFixedSize(23, 23);  //Tamaño fijo

              btnpause->setToolTip("Pause");


              btnrewind = new Button;
              btnrewind->SetIcon("rewind.svg");
              btnrewind->setIconSize(QSize(20, 20));
              btnrewind->setFixedSize(23, 23);  //30 29 Tamaño fijo
              btnrewind->setToolTip("Properties");

              btnforward = new Button;
              btnforward->SetIcon("forward.svg");
              btnforward->setIconSize(QSize(20, 20));
              btnforward->setFixedSize(23, 23);  //Tamaño fijo
              btnforward->setToolTip("Properties");


               slider = new Slider;


               connect(btnpause, &QPushButton::clicked, this, &Player::pauseMain);

               connect(btnrewind, &QPushButton::clicked, this, [this]() {
                   finishMix(false); mediamanager->rewind();
               });

               connect(btnforward, &QPushButton::clicked, this, [this]() {
                   finishMix(false); mediamanager->forward();
               });




               connect(slider, &QSlider::sliderPressed, this, [this]() {
                  m_userIsSeeking = true;

               });

               connect(slider, &QSlider::sliderReleased, this, [this]() {
                  m_userIsSeeking = false;

                   double percent = slider->value() / 1000.0;
                   finishMix(false);
                   mediamanager->seek(percent * m_duration);
               });




              layoutdown->addWidget(btnpause);
              layoutdown->addWidget(btnrewind);
              layoutdown->addWidget(btnforward);
              layoutdown->addWidget(slider,1);
              layoutdown->addStretch(); // todo a la izquierda


    //parte del tabplayer

     tabplayer = new TabPlayer(this);
     connect(tabplayer, &TabPlayer::playerFileNameChanged, this, [this](const QString &filename) {
         const QString name = filename.isEmpty() ? QStringLiteral("noname")
                                                 : QFileInfo(filename).completeBaseName();
         labeltitle->setText(QStringLiteral("Player [%1]").arg(name));
     });
     layout->addWidget(tabplayer);


}



Player::~Player(){}

void Player::setTitle(QString title)
{
    if (labeltitle) {
        labeltitle->setText(title);
    }
}

QString Player::title() const
{
    if (labeltitle) {
        return labeltitle->text();
    }
    return "";
}



void Player::bindMediaManager()
{
    MediaManager *manager = mediamanager;
     connect(mediamanager, &MediaManager::audioFrameUpdated,
               this, [this, manager](const AudioFrame &frame) {
         if (manager != mediamanager) return;

         if (vumeter) {
             if (mediamanager->isPlaying())
                 vumeter->setLevels(m_outgoingManager ? std::max(frame.left, m_outgoingLeft) : frame.left,
                                    m_outgoingManager ? std::max(frame.right, m_outgoingRight) : frame.right);
             else
                 vumeter->reset();
         }
         emit audioLevelsChanged(frame.left, frame.right);

         this->labeltiempo->setText(SecondToTime(mediamanager->isNetworkSource()
             ? frame.position : m_duration-frame.position));

         if (!m_userIsSeeking && m_duration > 0.0)
         {
             int value = static_cast<int>((frame.position / m_duration) * 1000.0);
             slider->setValue(value);
         }
         emit playbackProgressChanged(frame.position,m_duration,
                                      !mediamanager->isNetworkSource() && m_duration>0.0);
         tryFolderMix(frame.position);

       });

     connect(mediamanager, &MediaManager::networkLoadingChanged, this, [this, manager](bool loading) {
         if (manager != mediamanager) return;
         labeltiempo->setText(loading ? tr("Connecting...") : "00:00:00.00");
     });
     connect(mediamanager, &MediaManager::playbackError, this, [this, manager](const QString &message) {
         if (manager != mediamanager) return;
         if (m_sequenceContents) {
             advanceSequence();
             return;
         }
         stopMain();
         labelnombre->setText(message);
         labelnombre->setToolTip(message);
     });

            //emite el final
       connect(mediamanager, &MediaManager::playbackFinished,
               this, [this, manager]() {
           if (manager != mediamanager) return;
           finishMix(true);

           if (m_sequenceContents) {
               advanceSequence();
               return;
           }

           if (!currentItem)
               return;

           if (currentItem->isLoop()) {

               if (currentItem->advancesOnLoop()) {
                   AudioItemMaxi *item = currentItem;
                   stopMain();
                   playItem(item);
                   return;
               }

               // LOOP
               mediamanager->seek(currentItem->secondStart());
               mediamanager->play();
               return;
           }

           // guardar antes de parar
           AudioItemMaxi* finishedItem = currentItem;

           //  PRIORIDAD: NEXT + PURGE → repetir y NO borrar
           if (finishedItem->isPlayNext() && finishedItem->isPurge()) {

               this->stopMain();
               playItem(finishedItem);
               return;
           }

           // 1️⃣ Buscar el siguiente
           AudioItemMaxi* nextItem = nullptr;

           ContentsBase* contents = nullptr;
           QWidget* w = finishedItem;

           while (w) {
               contents = qobject_cast<ContentsBase*>(w);
               if (contents)
                   break;
               w = w->parentWidget();
           }

           if (contents) {
               nextItem = contents->findNextPlayItem(finishedItem);
           }

           // 2️⃣ parar
           this->stopMain();

           // 3️⃣ reproducir siguiente
           if (nextItem) {
               playItem(nextItem);
           }

           // 4️⃣ PURGE (solo si no era NEXT prioritario)
           if (finishedItem->isPurge()) {
               emit finishedItem->requestAutoDelete(finishedItem);
           }

        });






}

void Player::playItem(AudioItemMaxi *item)
{
    if (!item)
           return;
    finishMix(false);
    m_mixAttempted = false;


    if (currentItem && currentItem != item) {
          this->stopMain();
   }

       if (!mediamanager->setDevice(this->devicePlay())) {
           qWarning() << "Dispositivo de audio no disponible:" << this->devicePlay();
           return;
       }
       if (!item->preparePlayback()) {
           const auto *meteoItem=qobject_cast<AudioItemMeteoClockMaxi*>(item);
           qWarning().noquote() << "No se pudo preparar el audio:" << item->playbackName()
                                << (meteoItem ? meteoItem->error() : QString());
           return;
       }
       if (!item->loadPreparedPlayback(mediamanager)) {
           const auto *meteoItem=qobject_cast<AudioItemMeteoClockMaxi*>(item);
           qWarning().noquote() << "No se pudo cargar el audio:" << item->playbackName()
                                << (meteoItem ? meteoItem->error() : item->playbackPath());
           return;
       }
       mediamanager->seek(item->secondStart());
       mediamanager->play();
       item->setPlaying(true);
       item->playColor(true);

        // en caso de estar next activado se quita
       if (item->isPlayNext()) {
               item->setIsPlayNext(false);
           }

       currentItem = item;


       m_duration=item->second();
       emit playbackProgressChanged(mediamanager->getPosition(),m_duration,
                                    !mediamanager->isNetworkSource() && m_duration>0.0);
       startPlaybackLimit(item->playbackLimitSeconds());
       slider->setEnabled(!mediamanager->isNetworkSource());
       labelnombre->setText(item->playbackName());
       btnpause->SetIcon("Pausemini.svg");
}

bool Player::startSequentialPlayback(ContentsBase *contents, bool repeat)
{
    stopSequentialPlayback();
    if (!contents || !contents->layout)
        return false;
    if (currentItem)
        stopMain();
    m_sequenceContents=contents;
    m_sequenceRepeat=repeat;
    m_sequenceIndex=-1;
    if (playSequenceFrom(0))
        return true;
    m_sequenceContents.clear();
    return false;
}

void Player::stopSequentialPlayback()
{
    m_sequenceContents.clear();
    m_sequenceRepeat=false;
    m_sequenceIndex=-1;
    if (currentItem)
        stopMain();
}

bool Player::playSequenceFrom(int index)
{
    ContentsBase *contents=m_sequenceContents;
    if (!contents || !contents->layout)
        return false;
    const int count=contents->layout->count();
    if (count<=0)
        return false;
    for (int offset=0; offset<count; ++offset) {
        int candidate=index+offset;
        if (candidate>=count) {
            if (!m_sequenceRepeat)
                break;
            candidate%=count;
        }
        auto *item=qobject_cast<AudioItemMaxi*>(contents->layout->itemAt(candidate)->widget());
        if (!item)
            continue;
        playItem(item);
        if (currentItem==item) {
            m_sequenceIndex=candidate;
            return true;
        }
    }
    return false;
}

void Player::advanceSequence()
{
    ContentsBase *contents=m_sequenceContents;
    if (!contents || !contents->layout) {
        stopSequentialPlayback();
        emit sequentialPlaybackFinished();
        return;
    }
    const int count=contents->layout->count();
    const int next=m_sequenceIndex+1;
    if (count<=0 || (next>=count && !m_sequenceRepeat)) {
        stopMain();
        m_sequenceContents.clear();
        m_sequenceRepeat=false;
        m_sequenceIndex=-1;
        emit sequentialPlaybackFinished();
        return;
    }
    stopMain();
    if (playSequenceFrom(next))
        return;

    // No item could be started. Stop cleanly instead of spinning on an empty playlist.
    stopMain();
    m_sequenceContents.clear();
    m_sequenceRepeat=false;
    m_sequenceIndex=-1;
    emit sequentialPlaybackFinished();
}

bool Player::tryFolderMix(double position)
{
    auto *folder = qobject_cast<AudioItemFolderMaxi*>(currentItem);
    if (!folder || !folder->mixEnabled() || m_outgoingManager || m_pendingMixManager || m_mixAttempted
        || m_userIsSeeking || !mediamanager->isPlaying()) return false;
    const double remaining = m_duration - position;
    if (remaining <= 0.02 || remaining > folder->mixSeconds()) return false;
    AudioItemMaxi *next = folderTransitionTarget(folder);
    if (!next) return false;
    m_mixAttempted = true;
    auto *nextFolder = qobject_cast<AudioItemFolderMaxi*>(next);
    const auto oldSequence = nextFolder && nextFolder->m_sequence
        ? std::make_unique<AudioItemFolderMaxi::Sequence>(*nextFolder->m_sequence) : nullptr;
    const QString oldTrack = nextFolder ? nextFolder->m_currentTrack : QString();
    const double oldSeconds = next->second(), oldStart = next->secondStart();
    const auto restoreSelection = [&]() {
        if (!nextFolder || !oldSequence) return;
        if (nextFolder->m_sequence->revision == oldSequence->revision + 1)
            *nextFolder->m_sequence = *oldSequence;
        nextFolder->m_currentTrack = oldTrack;
        nextFolder->setSecond(oldSeconds); nextFolder->setTiempoFile(oldSeconds); nextFolder->setSecondStart(oldStart);
    };
    auto *incoming = new MediaManager(this);
    const bool prepared = incoming->setDevice(devicePlay()) && next->preparePlayback();
    if (prepared && next->isLiveStream()) {
        // Keep the current audio playing until the asynchronous radio connection is ready.
        incoming->setVolume(0.0f);
        m_pendingMixManager = incoming; m_pendingMixItem = next;
        connect(incoming, &MediaManager::networkLoadingChanged, this, [this, incoming](bool loading) {
            if (incoming == m_pendingMixManager && !loading) finishPendingMix();
        });
        connect(incoming, &MediaManager::playbackError, this, [this, incoming](const QString &) {
            if (incoming == m_pendingMixManager) cancelPendingMix();
        });
        if (!next->loadPreparedPlayback(incoming)) cancelPendingMix();
        return false;
    }
    if (!prepared || !next->loadPreparedPlayback(incoming)) {
        if (prepared) restoreSelection();
        delete incoming;
        return false;
    }
    const double available = incoming->getDuration() - next->secondStart();
    if (available <= 0.02) {
        restoreSelection();
        delete incoming;
        return false;
    }
    beginMix(next, incoming, remaining);
    return true;
}

void Player::beginMix(AudioItemMaxi *next, MediaManager *incoming, double remaining)
{
    auto *folder = qobject_cast<AudioItemFolderMaxi*>(currentItem);
    const double available = incoming->isNetworkSource() ? remaining * 2.0 : incoming->getDuration() - next->secondStart();
    incoming->seek(next->secondStart());
    incoming->setVolume(0.0f);
    m_outgoingManager = mediamanager;
    m_outgoingItem = currentItem;
    m_purgeOutgoing = next != currentItem && currentItem->isPurge();
    m_outgoingLeft = m_outgoingRight = -120.0f;
    if (next != currentItem) {
        currentItem->setPlaying(false); currentItem->playColor(false);
    }
    mediamanager = incoming;
    bindMediaManager();
    connect(m_outgoingManager, &MediaManager::playbackFinished, this, [this, outgoing = m_outgoingManager]() {
        if (outgoing == m_outgoingManager) finishMix(true);
    });
    connect(m_outgoingManager, &MediaManager::audioFrameUpdated, this,
        [this, outgoing = m_outgoingManager](const AudioFrame &frame) {
            if (outgoing != m_outgoingManager) return;
            m_outgoingLeft = frame.left; m_outgoingRight = frame.right;
        });
    currentItem = next;
    startPlaybackLimit(next->playbackLimitSeconds());
    next->setPlaying(true); next->playColor(true); next->setIsPlayNext(false);
    m_duration = incoming->getDuration();
    slider->setEnabled(!incoming->isNetworkSource());
    labelnombre->setText(next->playbackName());
    btnpause->SetIcon("Pausemini.svg");
    m_mixDurationMs = std::max(1, qRound(1000.0 * std::min({folder->mixSeconds(), remaining, available / 2.0})));
    m_mixElapsedMs = 0; m_mixProgress = 0.0f; m_mixAttempted = false;
    m_mixClock.restart();
    incoming->play();
    m_mixTimer->start();
}

void Player::finishPendingMix()
{
    auto *folder = qobject_cast<AudioItemFolderMaxi*>(currentItem);
    if (!folder || !m_pendingMixItem || !folder->mixEnabled()
        || folderTransitionTarget(folder) != m_pendingMixItem) { cancelPendingMix(); return; }
    if (!mediamanager->isPlaying()) return; // Pause also holds a connection that becomes ready.
    MediaManager *incoming = m_pendingMixManager;
    if (!incoming || incoming->isLoading()) return;
    incoming->play();
    const double remaining = m_duration - mediamanager->getPosition();
    if (!incoming->isPlaying() || remaining <= 0.02) { cancelPendingMix(); return; }
    auto *next = m_pendingMixItem.data();
    m_pendingMixManager = nullptr; m_pendingMixItem.clear();
    beginMix(next, incoming, remaining);
}

void Player::cancelPendingMix()
{
    if (!m_pendingMixManager) return;
    MediaManager *pending = m_pendingMixManager;
    m_pendingMixManager = nullptr; m_pendingMixItem.clear();
    pending->stop(); pending->deleteLater();
}

void Player::updateMix()
{
    if (!m_outgoingManager) return;
    const int elapsed = m_mixElapsedMs + int(m_mixClock.elapsed());
    m_mixProgress = std::clamp(float(elapsed) / m_mixDurationMs, 0.0f, 1.0f);
    const MixGains gains=equalPowerGains(m_mixProgress);
    mediamanager->setVolume(m_playerVolume * gains.incoming);
    m_outgoingManager->setVolume(m_playerVolume * gains.outgoing);
    if (elapsed >= m_mixDurationMs) finishMix(true);
}

void Player::finishMix(bool purge)
{
    cancelPendingMix();
    if (!m_outgoingManager) return;
    m_mixTimer->stop();
    MediaManager *outgoing = m_outgoingManager;
    m_outgoingManager = nullptr;
    outgoing->stop(); outgoing->deleteLater();
    m_mixProgress = 1.0f;
    mediamanager->setVolume(m_playerVolume);
    auto item = m_outgoingItem;
    m_outgoingItem.clear();
    if (purge && m_purgeOutgoing && item && item != currentItem)
        emit item->requestAutoDelete(item);
    m_purgeOutgoing = false;
}

void Player::pauseMain()
{
    if (!currentItem)
            return;

        if (mediamanager->isPlaying()) {
            mediamanager->pause();
            pausePlaybackLimit();
            if (m_outgoingManager) {
                m_outgoingManager->pause();
                m_mixElapsedMs += int(m_mixClock.elapsed());
                m_mixTimer->stop();
            }
            vumeter->reset();
            btnpause->SetIcon("Playmini.svg");   // opcional: cambia icono a play

        } else {
            mediamanager->play();
            resumePlaybackLimit();
            if (m_pendingMixManager && !m_pendingMixManager->isLoading()) finishPendingMix();
            if (m_outgoingManager) {
                m_outgoingManager->play();
                m_mixClock.restart(); m_mixTimer->start();
            }
            btnpause->SetIcon("Pausemini.svg");  // opcional: vuelve icono pause

        }
}

void Player::dragEnterEvent(QDragEnterEvent *event)
{
    if (!TabPlayer::droppedPlayerFile(event->mimeData()).isEmpty()) event->acceptProposedAction();
    else event->ignore();
}

void Player::dropEvent(QDropEvent *event)
{
    if (tabplayer->loadDroppedPlayer(event->mimeData())) event->acceptProposedAction();
    else event->ignore();
}

void Player::stopMain()
{
    if (m_playbackLimitTimer)
        m_playbackLimitTimer->stop();
    m_playbackLimitRemainingMs=0;
    finishMix(false);

    if (!currentItem)
           return;

        mediamanager->stop();
        mediamanager->seek(0.0);
        vumeter->reset();


        if (currentItem) {  // <-- protección extra
            currentItem->setPlaying(false);
            currentItem->playColor(false);
        }

        currentItem = nullptr;

        labelnombre->setText("");
        labeltiempo->setText("00:00:00.00");
        emit playbackProgressChanged(0.0,0.0,false);

}

bool Player::seekPlaybackPosition(double seconds)
{
    if (!currentItem || mediamanager->isNetworkSource() || m_duration<=0.0
        || !std::isfinite(seconds))
        return false;

    finishMix(false);
    mediamanager->seek(std::clamp(seconds,0.0,m_duration));
    return true;
}

void Player::startPlaybackLimit(double seconds)
{
    if (!m_playbackLimitTimer)
        return;
    m_playbackLimitTimer->stop();
    m_playbackLimitRemainingMs=seconds>0.0 ? qRound64(seconds*1000.0) : 0;
    if (m_playbackLimitRemainingMs<=0)
        return;
    m_playbackLimitClock.restart();
    m_playbackLimitTimer->start(static_cast<int>(m_playbackLimitRemainingMs));
}

void Player::pausePlaybackLimit()
{
    if (!m_playbackLimitTimer || !m_playbackLimitTimer->isActive())
        return;
    m_playbackLimitRemainingMs=qMax<qint64>(0,m_playbackLimitRemainingMs-m_playbackLimitClock.elapsed());
    m_playbackLimitTimer->stop();
}

void Player::resumePlaybackLimit()
{
    if (!m_playbackLimitTimer || m_playbackLimitRemainingMs<=0
        || !currentItem || currentItem->playbackLimitSeconds()<=0.0)
        return;
    m_playbackLimitClock.restart();
    m_playbackLimitTimer->start(static_cast<int>(m_playbackLimitRemainingMs));
}


bool Player::setVolume(float volume)
{
    if (!std::isfinite(volume)) return false;
    m_playerVolume = std::clamp(volume, 0.0f, 1.0f);
    const MixGains gains=equalPowerGains(m_mixProgress);
    bool result = mediamanager->setVolume(m_playerVolume * gains.incoming);
    if (m_outgoingManager)
        result = m_outgoingManager->setVolume(m_playerVolume * gains.outgoing) && result;
    return result;
}

float Player::volume() const
{
    return m_playerVolume;
}

int Player::devicePlay() const {return m_deviceplay;}

void Player::setDevicePlay(int device) {m_deviceplay = device;}

int Player::deviceCue() const {return m_devicecue;}

void Player::setDeviceCue(int device) {m_devicecue = device;}




QString Player::SecondToTime(double segundos){

    if (segundos < 0) {
          return "00:00:00.00";
        }

        // Convertimos a milisegundos para mayor precisión
      //  qint64 totalMilliseconds = static_cast<qint64>(segundos * 1000.0);
        qint64 totalMilliseconds = qRound64(segundos * 1000.0);

        int horas = totalMilliseconds / 3600000;
        int minutos = (totalMilliseconds % 3600000) / 60000;
        int seg = (totalMilliseconds % 60000) / 1000;
        int centesimas = (totalMilliseconds % 1000) / 10;

        QString tiempo = QString("%1:%2:%3.%4")
                .arg(horas, 2, 10, QChar('0'))
                .arg(minutos, 2, 10, QChar('0'))
                .arg(seg, 2, 10, QChar('0'))
                .arg(centesimas, 2, 10, QChar('0'));

        return tiempo;



}

float Player::committedVolume() const
{
    return frameoptionsplayer ? frameoptionsplayer->committedVolume() : volume();
}
