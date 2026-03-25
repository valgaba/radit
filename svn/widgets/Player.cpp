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
//#include "widgets/container.h"





Player::Player(QWidget *parent) : Frame(parent) {

  //  this->setStyleSheet("background-color: none;");


       mediamanager = new MediaManager(this);

     connect(mediamanager, &MediaManager::audioFrameUpdated,
               this, [this](const AudioFrame &frame) {



       });


       connect(mediamanager, &MediaManager::playbackFinished,
               this, [this]() {
                   mediamanager->stop();
                   mediamanager->seek(0.0);
                   currentItem = nullptr;
               });





      layout = new QVBoxLayout(this);  // layout general
      layout->setContentsMargins(0, 0, 0, 0);
      layout->setSpacing(0);

      framebarra = new Frame(this);
      frametop = new Frame(this);
      framecenter = new Frame(this);
      framedown = new Frame(this);
    //  frametab = new Frame(this);

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
      layoutdown->setSpacing(0);
     // layouttab->setSpacing(0);

      layout->addWidget(framebarra);
      layout->addWidget(frametop);
      layout->addWidget(framecenter);
      layout->addWidget(framedown);
      //layout->addWidget(frametab);

      // partes fijas
      framebarra->setFixedHeight(25);
      frametop->setFixedHeight(30);
      framecenter->setFixedHeight(50);
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

      /* if (currentItem == item) {
           mediamanager->stop();
           mediamanager->seek(0.0);
           mediamanager->play();
           return;
       }*/

    if (currentItem && currentItem != item) {
        mediamanager->stop();
        mediamanager->seek(0.0);
   }

       mediamanager->setDevice(-1);
       mediamanager->loadFile(item->filePath());
       mediamanager->play();

       currentItem = item;
}

void Player::pauseMain()
{
    if (!currentItem)
        return;

    if (mediamanager->isPlaying()) {
        mediamanager->pause();
    }
}

void Player::stopMain()
{
    if (!currentItem)
        return;

    mediamanager->stop();
    mediamanager->seek(0.0);
    currentItem = nullptr;
}


