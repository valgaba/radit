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



#include "widgets/AudioItemMini.h"

AudioItemMini::AudioItemMini(QWidget *parent):AudioItem(parent){


   this->setObjectName("AudioItemMini"); //para qss
   //this->setMinimumSize(100,30); // alto del item


   layout = new QHBoxLayout;
   layout->setContentsMargins(0, 0, 0, 0);
   layout->setSpacing(0); // espacios entre  item dentro del contenedor
   this->setLayout(layout);


   QString horaActual = QDateTime::currentDateTime().toString("HH:mm:ss");

   hora =     new QLabel(horaActual);
   nombre =   new QLabel;
   duracion = new QLabel("00:03:15");



   btnproperties = new Button;
   btnproperties->SetIcon("GuiTabMenu.svg");
   btnproperties->setFixedSize(30, 29);  //Tamaño fijo

   btnproperties->setToolTip("Properties");

   // Ajustar el QLabel al tamaño de su texto
   hora->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   hora->adjustSize();
   duracion->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   duracion->adjustSize();
   btnproperties->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   btnproperties->adjustSize();


   layout->addWidget(hora);
   layout->addWidget(nombre);
   layout->addWidget(duracion);
   layout->addWidget(btnproperties);


}



AudioItemMini::~AudioItemMini(){}

void AudioItemMini::setNameFile(const QString &NameFile) const{

     nombre->setText(NameFile);  // Asigna el texto al QLabel


}






