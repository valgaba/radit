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



#include "widgets/AudioItemFileMini.h"

AudioItemFileMini::AudioItemFileMini(QWidget *parent):AudioItemMini(parent){


   this->setObjectName("AudioItemFileMini"); //para qss
  // this->setMinimumSize(100,30); // alto del item
   this->setNameFile(this->metaObject()->className());
}



AudioItemFileMini::~AudioItemFileMini(){}





//***************** cuando hacemos paste hacemos una copia del objeto
AudioItemMini* AudioItemFileMini::copy(QWidget* newParent) const  {

   AudioItemFileMini* audioitem = new AudioItemFileMini(newParent);

  /* audioitem->hora->setText(this->hora->text());
   audioitem->duracion->setText(this->duracion->text());
   audioitem->nombre->setText(this->nombre->text());*/

   audioitem->setToolTip(this->toolTip());

   return audioitem;
}









