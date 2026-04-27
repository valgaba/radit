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
#include "widgets/Player.h"
#include "widgets/contentsbase.h"
//#include "widgets/container.h"





Player::Player(QWidget *parent) : Frame(parent) {

  //  this->setStyleSheet("background-color: none;");


       mediamanager = new MediaManager(this);

     connect(mediamanager, &MediaManager::audioFrameUpdated,
               this, [this](const AudioFrame &frame) {

         this->labeltiempo->setText(SecondToTime(m_duration-frame.position)); //cuenta atras del tiempo

         if (!m_userIsSeeking && m_duration > 0.0)
         {
             int value = static_cast<int>((frame.position / m_duration) * 1000.0);
             slider->setValue(value);
         }

       });


       connect(mediamanager, &MediaManager::playbackFinished,
               this, [this]() {

           if (!currentItem)
                  return;

              if (currentItem->isLoop()) {

                  // LOOP
                  mediamanager->seek(currentItem->secondStart());
                  mediamanager->play();
                  return;
              }

              // guardar antes de parar
              AudioItemMaxi* finishedItem = currentItem;

              this->stopMain();

              // PURGE
              if (finishedItem->isPurge()) {
                  emit finishedItem->requestAutoDelete(finishedItem);
              }

              //  PLAY NEXT
              ContentsBase* contents = nullptr;
              QWidget* w = finishedItem;

              while (w) {
                  contents = qobject_cast<ContentsBase*>(w);
                  if (contents)
                      break;
                  w = w->parentWidget();
              }

              if (contents) {
                  AudioItemMaxi* nextItem = contents->findNextPlayItem(finishedItem);

                  if (nextItem) {
                      nextItem->setIsPlayNext(false);
                      playItem(nextItem);
                  }
              }
               });





      layout = new QVBoxLayout(this);  // layout general
      layout->setContentsMargins(0, 0, 0, 0);
      layout->setSpacing(0);

      framebarra = new Frame(this);
      frametop = new Frame(this);
      framecenter = new Frame(this);
      framedown = new Frame(this);

      //personalizamos un poco la barra
        //framecenter->setStyleSheet("background-color: rgb(38, 44, 59);");
        framecenter->setStyleSheet("background-color: #181e2c;");
        frametop->setStyleSheet("background-color: #181e2c;");
        framedown->setStyleSheet("background-color: #181e2c;");


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
      layouttop->setSpacing(0);
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
      frametop->setFixedHeight(25);
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
          QMessageBox::StandardButton reply;
          reply = QMessageBox::question(
              this,
              "Cerrar Player",
              "¿Seguro que quieres cerrar este player?",
              QMessageBox::Yes | QMessageBox::No
          );

          if (reply == QMessageBox::Yes) {
              this->deleteLater();
          }
      });



    //parte central
      btnstop = new Button(this);
      btnstop->SetIcon("Stop.svg");
      btnstop->setIconSize(QSize(40, 50));  // ajusta al tamaño que quieras
      btnstop->setFixedSize(50, 30);  //Tamaño fijo
      btnstop->setToolTip("Stop");

      labelnombre = new Label;
      labelnombre->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
      labeltiempo = new Label;
      labeltiempo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
      labeltiempo->setFixedWidth(125);   // Fija solo el ancho
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

               connect(btnrewind, &QPushButton::clicked,
                       mediamanager, &MediaManager::rewind);

               connect(btnforward, &QPushButton::clicked,
                       mediamanager, &MediaManager::forward);




               connect(slider, &QSlider::sliderPressed, this, [this]() {
                  m_userIsSeeking = true;

               });

               connect(slider, &QSlider::sliderReleased, this, [this]() {
                  m_userIsSeeking = false;

                   double percent = slider->value() / 1000.0;
                   mediamanager->seek(percent * m_duration);
               });




              layoutdown->addWidget(btnpause);
              layoutdown->addWidget(btnrewind);
              layoutdown->addWidget(btnforward);
              layoutdown->addWidget(slider,1);
              layoutdown->addStretch(); // todo a la izquierda


    //parte del tabplayer

     tabplayer = new TabPlayer(this);
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



void Player::playItem(AudioItemMaxi *item)
{
    if (!item)
           return;


    if (currentItem && currentItem != item) {
       // mediamanager->stop();
       // mediamanager->seek(0.0);
       //  currentItem->setPlaying(false);
        this->stopMain();
   }

       mediamanager->setDevice(1);
       mediamanager->loadFile(item->filePath());
       mediamanager->seek(item->secondStart());
       mediamanager->play();
       item->setPlaying(true);
       currentItem = item;


       m_duration=item->second();
       labelnombre->setText(item->nameFile());
       btnpause->SetIcon("Pausemini.svg");
}

void Player::pauseMain()
{
    if (!currentItem)
            return;

        if (mediamanager->isPlaying()) {
            mediamanager->pause();
            btnpause->SetIcon("Playmini.svg");   // opcional: cambia icono a play

        } else {
            mediamanager->play();
            btnpause->SetIcon("Pausemini.svg");  // opcional: vuelve icono pause

        }
}

void Player::stopMain()
{

    if (!currentItem)
           return;

        mediamanager->stop();
        mediamanager->seek(0.0);


        if (currentItem) {  // <-- protección extra
            currentItem->setPlaying(false);
        }

        currentItem = nullptr;

        labelnombre->setText("");
        labeltiempo->setText("00:00:00.00");
}

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
