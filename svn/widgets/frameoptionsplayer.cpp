/* This file is part of Radit.
   Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Iradit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QComboBox>
#include <QLabel>

#include "widgets/frameoptionsplayer.h"
#include "widgets/label.h"





FrameOptionsPlayer::FrameOptionsPlayer(QWidget *parent):Frame(parent){

    this->setObjectName("FrameOptionsPlayer"); //para qss
    this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    this->setFixedSize(300, 250);

    layout = new QVBoxLayout(this);  // layout general
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);


    frametop = new Frame(this);
    framecenter = new Frame(this);
    framedown = new Frame(this);

    framecenter->setObjectName("framecenter");
    frametop->setObjectName("frametop");
    framedown->setObjectName("framedown");


    layouttop = new QHBoxLayout(frametop);
    layoutcenter = new QFormLayout(framecenter);
    layoutdown = new QHBoxLayout(framedown);

    layoutdown->setContentsMargins(1, 1, 1, 1);
    layoutdown->setSpacing(10); // espacios entre  item dentro del contenedor

    layouttop->setContentsMargins(0, 0, 0, 0);
    layouttop->setSpacing(0); // espacios entre  item dentro del contenedor



    layout->addWidget(frametop);
    layout->addWidget(framecenter);
    layout->addWidget(framedown);

    layout->setStretch(0, 1); // arriba
    layout->setStretch(1, 4); // centro
    layout->setStretch(2, 1); // abajo

    frametop->setFixedHeight(23);

    //parte alta

    frametop->setStyleSheet("background-color: #3E3E5C; border: none;");


     Label * labeltext = new Label(this);
     labeltext->setMaximumHeight(QWIDGETSIZE_MAX);
     labeltext->setWordWrap(false);
     QFont font;
          font.setPointSize(10);
          font.setBold(true);
          labeltext->setFont(font);

     labeltext->setText("Player options");
     layouttop->addWidget(labeltext);




    //
   // layout->setContentsMargins(10, 10, 10, 10);
   // layout->setSpacing(8);


    Label *labelplay= new Label(this);

   labelplay->setMaximumHeight(QWIDGETSIZE_MAX);
   labelplay->setWordWrap(false);
   labelplay->setText("Play Out");

    QComboBox *comboplay = new QComboBox(this);
    comboplay->addItems({
        "España",
        "Francia",
        "Italia"
    });

    layoutcenter->addRow(labelplay, comboplay);

    Label *labelcue= new Label(this);

   labelcue->setMaximumHeight(QWIDGETSIZE_MAX);
   labelcue->setWordWrap(false);
   labelcue->setText("Play Cue");

    QComboBox *combocue = new QComboBox(this);
    combocue->addItems({
        "España",
        "Francia",
        "Italia"
    });

    layoutcenter->addRow(labelcue, combocue);









    //parte baja

    btncancel = new Button(this);
    btncancel->setText("Cancel");

    btncancel->setStyleSheet(
        "QPushButton {"
        "    padding: 4px 12px;"
        "}"
    );


    btnacept = new Button(this);
    btnacept->setText("Acept");

    btnacept->setStyleSheet(
        "QPushButton {"
        "    padding: 4px 12px;"
        "}"
    );

    layoutdown->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum)); //espaciador

    layoutdown->addWidget(btncancel);
    layoutdown->addWidget(btnacept);


    // Cancelar -> ocultar el panel
    connect(btncancel, &QPushButton::clicked,
            this, &FrameOptionsPlayer::hide);



    // aceptar -> ocultar el panel
    connect(btnacept, &QPushButton::clicked,
            this, &FrameOptionsPlayer::hide);





}







FrameOptionsPlayer::~FrameOptionsPlayer(){}








