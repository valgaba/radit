/* This file is part of Radit.
   Copyright 2023, Victor Algaba <victorengine@gmail.com> www.radit.org

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



#include "widgets/View.h"


View::View(QGraphicsScene *scene) : QGraphicsView(scene) {


    this->setObjectName("View"); //para qss
   // this->setStyleSheet("background-color: black;");



 }

View::~View(){}




void View::wheelEvent(QWheelEvent *event){
    if (event->modifiers() & Qt::ControlModifier) { //pulsa tecla control para el escalado
        qreal scaleFactor = 1.1; // Factor de escala predeterminado
        if (event->angleDelta().y() < 0) {
            // Si la rueda se desplaza hacia abajo, reducimos la escala
            scaleFactor = 1.0 / scaleFactor;
        }
        scale(scaleFactor, scaleFactor); // Aplicar la escala
        event->accept();
    } else {
        // Pasa el evento de la rueda del ratón a la clase base
        QGraphicsView::wheelEvent(event);
    }
}
