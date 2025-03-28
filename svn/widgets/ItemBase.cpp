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



#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QDebug>



#include "widgets/ItemBase.h"

ItemBase::ItemBase(QWidget *parent):QFrame(parent){


    this->setObjectName("ItemBase"); //para qss

}



ItemBase::~ItemBase(){}



void ItemBase::mousePressEvent(QMouseEvent *event){

    if ((event->button() == Qt::LeftButton)){
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setText("AudioItem");
        drag->setMimeData(mimeData);


        // Evita el problema de QPixmap::scaled: Pixmap is a null pixmap
        // Asignar un QPixmap válido (incluso si es transparente)
        QPixmap pixmap(1, 1); // Imagen de 1x1 píxel
        pixmap.fill(Qt::transparent); // Imagen transparente
        drag->setPixmap(pixmap);




        Qt::DropAction dropAction = drag->exec(Qt::MoveAction);

        // Delete later, if there is no drop event.
        if(  dropAction == Qt::IgnoreAction ){
            drag->deleteLater();
            //mimeData->deleteLater();
            // qDebug() <<"papanata";
        }

    }

}



//***********************************************

//***********************************************
void ItemBase::setdayweek(int dayweek){
    this->dayweek=dayweek;
}

void ItemBase::sethour(int hour){
    this->hour=hour;
}
void ItemBase::setminute(int minute){
    this->minute=minute;

}
void ItemBase::setsecond(int second){
    this->second=second;

}

void ItemBase::setfilePath(QString filePath){
       this->filePath=filePath;

}

void ItemBase::setfilesecond(double filesecond){
    this->filesecond=filesecond;
}


int ItemBase::getdayweek()const {
    return this->dayweek;
}

int ItemBase::gethour()const{
    return this->hour;
}
int ItemBase::getminute()const{
    return this->minute;
}

int ItemBase::getsecond()const{
    return this->second;
}

QString ItemBase::getfilePath()const{
    return this->filePath;
}
double ItemBase::getfilesecond()const{
    return this->filesecond;
}

