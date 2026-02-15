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
#include <QIcon>


#include "widgets/button.h"

Button::Button(QWidget *parent):QPushButton(parent){

    this->setObjectName("Button"); //para qss

    QFont font;
    font.setPointSize(12);
    font.setBold(false);

    this->setFont(font);


    this->setCheckable(false);
   // this->setFlat(true);
    this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor

 }



Button::~Button(){}


void Button::SetIcon(QString file){
    QIcon icon;

   icon.addFile(":/icons/"+file, QSize(), QIcon::Normal, QIcon::Off);
    this->setIcon(icon);
    this->setIconSize(QSize(25, 25));   // tamaño del icono

}



