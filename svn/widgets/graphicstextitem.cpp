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


#include <QFont>

#include "graphicstextitem.h"



GraphicsTextItem::GraphicsTextItem(QGraphicsItem* parent):QGraphicsTextItem(parent){

    this->setObjectName("GraphicsTextItem"); //para qss

    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);


}

GraphicsTextItem::~GraphicsTextItem(){}


void GraphicsTextItem::wheelEvent(QGraphicsSceneWheelEvent *event){
    if (isSelected()) {
        // Obtener la cantidad de desplazamiento de la rueda del ratón
        qreal delta = event->delta();

        // Calcular la escala de la fuente en función del desplazamiento de la rueda del ratón
        qreal scaleFactor = (delta > 0) ? 1.1 : 0.9;

        // Ajustar el tamaño de fuente del texto
        QFont font = this->font();
        font.setPointSizeF(font.pointSizeF() * scaleFactor);

        setFont(font);

        event->accept(); // Indicar que el evento ha sido manejado
    } else {
        QGraphicsTextItem::wheelEvent(event);
    }
}





