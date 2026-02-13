/* This file is part of Radit.
  Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

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
#include <QDateTime>
#include <QSpacerItem>



#include "widgets/AudioItemMaxi.h"

AudioItemMaxi::AudioItemMaxi(QWidget *parent):AudioItem(parent){


   this->setObjectName("AudioItemMaxi"); //para qss
   this->setFixedHeight(110); //alto del item fijo


   layout = new QVBoxLayout; //layout general
   layout->setContentsMargins(0, 0, 0, 0);
   layout->setSpacing(0); // espacios entre  item dentro del contenedor
   this->setLayout(layout);

   // diviedimos en tres partes
       framecolor = new Frame;
       framecolor->setFixedHeight(10);
       framecolor->setFixedWidth(50);
       framecolor->setFrameShape(QFrame::NoFrame);
       framecolor->setFrameShadow(QFrame::Plain);
       framecolor->setStyleSheet("background-color: red;");
       //framecolor->setVisible(false);



       frametop = new Frame;
       framecenter = new Frame;
       framedown = new Frame;

       //layou de las zonas
       layouttop = new QHBoxLayout;
       layoutdown =new QHBoxLayout;
       layoutcenter = new QHBoxLayout;


      layoutcenterleft = new QHBoxLayout;
      layoutcenterright = new QHBoxLayout ;



       layouttop->setContentsMargins(0, 0, 0, 0);
       layoutdown->setContentsMargins(0, 0, 0, 0);
       layoutcenter->setContentsMargins(0, 0, 0, 0);
       layoutcenter->setSpacing(0); // espacios entre  item dentro del contenedor

       layoutcenterleft->setContentsMargins(0, 0, 0, 0);
       layoutcenterright->setContentsMargins(0, 0, 0, 0);
       layoutcenterleft->setSpacing(0); // espacios entre  item dentro del contenedor


       frametop->setLayout(layouttop);
       framecenter->setLayout(layoutcenter);
       framedown->setLayout(layoutdown);



       // la zona centro se divide en dos partes con sus correspondientes layou
       framecenterleft = new Frame;
       framecenterright = new Frame;

       framecenterleft->setLayout(layoutcenterleft);
       framecenterright->setLayout(layoutcenterright);

       layoutcenter->addWidget(framecenterleft);
       layoutcenter->addWidget(framecenterright,1); // solo se redimensiona este


//*****************************

        btnproperties = new Button;
        btnproperties->SetIcon("GuiTabMenu.svg");
        btnproperties->setFixedSize(30, 29);  //Tamaño fijo
        btnproperties->setToolTip("Properties");


        layouttop->addWidget(framecolor, 0, Qt::AlignTop);
        layouttop->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum));
        layouttop->addWidget(btnproperties);


        btnplaycue = new Button;
        btnplaycue->SetIcon("PlayCue.svg");
        btnplaycue->setFixedSize(30, 29);  //Tamaño fijo
        btnplaycue->setToolTip("Properties");
        layoutdown->addWidget(btnplaycue);
        layoutdown->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum));


        connect(btnplaycue, &QPushButton::clicked, this, [=](){
            framecolor->setVisible(!framecolor->isVisible()); // alterna visibilidad
        });




        btnplay = new Button;
        btnplay->SetIcon("Play.svg");
        btnplay->setIconSize(QSize(30, 50));  // ajusta al tamaño que quieras
        btnplay->setFixedSize(60, 35);  //Tamaño fijo
        btnplay->setToolTip("Properties");
        layoutcenterleft->addWidget(btnplay);









        //añadimos al principal
       // layout->addWidget(framecolor);
        layout->addWidget(frametop,1);
        layout->addWidget(framecenter,3);
        layout->addWidget(framedown,1);


}



AudioItemMaxi::~AudioItemMaxi(){}

void AudioItemMaxi::setNameFile(const QString &NameFile) const{


}






