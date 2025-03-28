/* This file is part of IRadit.
   Copyright 2020, Victor Algaba <victor@radit.org> www.radit.org

   IRadit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Iradit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Iradit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>




#include "widgets//slider.h"





Slider::Slider(QWidget *parent):QSlider(parent){


    this->setObjectName("Slider"); //para qss
    this->setOrientation(Qt::Horizontal);
    this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor

    this->setMinimumSize(QSize(10, 5));      //minimo que ocupa el itemaudio
    this->setMaximumSize(QSize(10000, 20)); //maxima que ocupa el itenaudio


    QSizePolicy sizePolicy; //para ocupar todo el espacio
    sizePolicy.setHorizontalStretch(1);
    this->setSizePolicy(sizePolicy);

    this->setSingleStep(0);  // Desactiva el movimiento con la rueda del ratón



}







Slider::~Slider(){}













