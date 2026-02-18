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
       layoutcenterright->setSpacing(0); // espacios entre  item dentro del contenedor



       frametop->setLayout(layouttop);
       framecenter->setLayout(layoutcenter);
       framedown->setLayout(layoutdown);



       // la zona centro se divide en dos partes con sus correspondientes layou
       framecenterleft = new Frame;
       framecenterright = new Frame;

       framecenterleft->setLayout(layoutcenterleft);
       framecenterright->setLayout(layoutcenterright);

       framecenterleft->setStyleSheet("Frame { border: none;}"); //quitamos bordes de los layou
       framecenterright->setStyleSheet("Frame { border: none;}");


       layoutcenter->addWidget(framecenterleft);
       layoutcenter->addWidget(framecenterright,1); // solo se redimensiona este


//****************Botonera parte alta*************

       framecolor = new FrameColorItemMax;

       btnselect = new Button;
      // btnproperties->SetIcon("GuiTabMenu.svg");
       btnselect->setFixedSize(30, 29);  //Tamaño fijo
       btnselect->setToolTip("Select item");


        btnproperties = new Button;
        btnproperties->SetIcon("more.svg");
        btnproperties->setFixedSize(30, 29);  //Tamaño fijo
        btnproperties->setToolTip("Properties");

        btndelete = new Button;
        btndelete->SetIcon("Remove.svg");
        btndelete->setFixedSize(30, 29);  //Tamaño fijo
        btndelete->setToolTip("Delete item");


        btnloop = new Button;
        btnloop->SetIcon("Loop.svg");
        btnloop->setIconSize(QSize(20, 20));   // no termina de gustarme el tamaño del icono por defecto
        btnloop->setFixedSize(30, 29);  //Tamaño fijo
        btnloop->setToolTip("Repeat the item indefinitely.");

        btnpurge = new Button;
        btnpurge->SetIcon("Purge.svg");
        btnpurge->setIconSize(QSize(20, 20));   // no termina de gustarme el tamaño del icono por defecto
        btnpurge->setFixedSize(30, 29);  //Tamaño fijo
        btnpurge->setToolTip("Delete the item once it has been played.");

        btnnext = new Button;
        btnnext->SetIcon("Next.svg");
        btnnext->setIconSize(QSize(20, 20));   // no termina de gustarme el tamaño del icono por defecto
        btnnext->setFixedSize(30, 29);  //Tamaño fijo
        btnnext->setToolTip("Play the item once the previous one has finished");


        connect(btnnext, &QPushButton::clicked, this, [=](){
             this->setIsPlayNext(!this->isPlayNext());
        });

        connect(btnpurge, &QPushButton::clicked, this, [=](){
             this->setIsPurge(!this->isPurge());
        });

        connect(btnloop, &QPushButton::clicked, this, [=](){
             this->setIsLoop(!this->isLoop());
        });


        connect(btndelete, &QPushButton::clicked,
                this, &AudioItemMaxi::onDeleteClicked);





        layouttop->addWidget(framecolor, 0, Qt::AlignTop); //añadimos un color al item
        layouttop->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum)); //espaciador

        // añadimos botones a la parte alta
        layouttop->addWidget(btnnext);
        layouttop->addWidget(btnpurge);
        layouttop->addWidget(btnloop);
        layouttop->addWidget(btndelete);
        layouttop->addWidget(btnproperties);
        layouttop->addWidget(btnselect);



        // botonera parte centro**********************************

        btnplay = new Button;
        btnplay->SetIcon("playpause2.svg");
        btnplay->setIconSize(QSize(50, 60));  // ajusta al tamaño que quieras
        btnplay->setFixedSize(60, 35);  //Tamaño fijo
        btnplay->setToolTip("Properties");



        labelnombre = new Label;
        labelnombre->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        labelnombre->setText("fichero.....dfdfdfdfdfdfdfdfdfdfdf.....");

        labeltiempo = new Label;
        labeltiempo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        labeltiempo->setFixedWidth(125);   // Fija solo el ancho
        // Cambiar tamaño de fuente
        QFont font = labeltiempo->font();
        font.setPointSize(16);
        font.setBold(true);
        labeltiempo->setFont(font);


        labeltiempo->setText("00:03:12.00");

        labeltiempo->setFixedHeight(45);
        labelnombre->setFixedHeight(45);

        layoutcenterleft->addWidget(btnplay);
        layoutcenterright->addWidget(labelnombre);
        layoutcenterright->addWidget(labeltiempo);







///***************** botonera parte baja ****************
        btnplaycue = new Button;
        btnplaycue->SetIcon("playpause.svg");
        btnplaycue->setIconSize(QSize(35, 35));   // no termina de gustarme el tamaño del icono por defecto
        btnplaycue->setFixedSize(30, 29);  //Tamaño fijo
        btnplaycue->setToolTip("Properties");


        btnrewind = new Button;
        btnrewind->SetIcon("rewind.svg");
        btnrewind->setFixedSize(30, 29);  //Tamaño fijo
        btnrewind->setToolTip("Properties");

        btnforward = new Button;
        btnforward->SetIcon("forward.svg");
        btnforward->setFixedSize(30, 29);  //Tamaño fijo
        btnforward->setToolTip("Properties");


         slider = new Slider;
        // slider->setFixedHeight(29);


        layoutdown->addWidget(btnplaycue);
        layoutdown->addWidget(btnrewind);
        layoutdown->addWidget(btnforward);
        layoutdown->addWidget(slider,1);

       // layoutdown->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum));


        connect(btnplaycue, &QPushButton::clicked, this, [=](){
            framecolor->setVisible(!framecolor->isVisible()); // alterna visibilidad
        });











        //añadimos al principal
       // layout->addWidget(framecolor);
        layout->addWidget(frametop,1);
        layout->addWidget(framecenter,3);
        layout->addWidget(framedown,1);


}



AudioItemMaxi::~AudioItemMaxi(){}

void AudioItemMaxi::setNameFile(const QString &NameFile) const{


}

//***************************************
void AudioItemMaxi::onDeleteClicked(){  // pulsamos boton borrar

    emit requestDelete(this);  //mandamos señal borrar
}



