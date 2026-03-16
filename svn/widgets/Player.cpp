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

#include "widgets/Player.h"





Player::Player(QWidget *parent) : Frame(parent) {

  //  this->setStyleSheet("background-color: none;");

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
      framecenter->setFixedHeight(60);
      framedown->setFixedHeight(30);

      // parte flexible
      layout->setStretch(4, 1); // frametab

      // pintar frametop de azul
      framebarra->setStyleSheet("background-color: #4e4d7a;");
    // this->setStyleSheet("background-color: #343434;");


      tabplayer = new TabPlayer;



      for (int i = 0; i < 4; ++i) {
          Container *container = new Container;
       //  container->setStyleSheet("background-color: #282020;");
          tabplayer->addTab(container, "lista " + QString::number(i + 1));
      }

    // layouttab->addWidget(tabplayer);
      layout->addWidget(tabplayer);


}



Player::~Player(){}

