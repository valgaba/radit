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
#include <QPainter>
#include <QVBoxLayout>
#include <QResizeEvent>


#include "widgets/ExternalWindow.h"





ExternalWindow ::ExternalWindow (QWidget *parent):QWidget(parent){

    this->setObjectName("ExternalWindow"); //para qss

    setWindowTitle("External Window");
    this->resize(400, 400);
   // this->setFixedSize(400, 300);
   // this->setStyleSheet("QFrame { background-color: black; border: 0px; }");
   // this->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);

    QVBoxLayout *layout = new QVBoxLayout(this);
      layout->setContentsMargins(0, 0, 0, 0);
      label = new QLabel;
     // label->setAlignment(Qt::AlignCenter);
     // label->setScaledContents(true); // Escalar el contenido del QLabel

     // layout->addWidget(label);

      layout->addWidget(label, 1, Qt::AlignCenter);



}



ExternalWindow::~ExternalWindow (){}

void ExternalWindow:: renderFrame(const QPixmap &pixmap){

    QSize windowSize = this->size();

    // Escalar el QPixmap para que coincida con el tamaño de la ventana
    QPixmap scaledPixmap = pixmap.scaled(windowSize, Qt::KeepAspectRatio,Qt::SmoothTransformation );
    label->setPixmap(scaledPixmap);




}

void ExternalWindow::resizeEvent(QResizeEvent *event) {


}




