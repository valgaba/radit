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

#include <QWheelEvent>



#include "widgets/Scene.h"
#include "widgets/graphicstextitem.h"





Scene::Scene(QObject *parent):QGraphicsScene(parent){

    this->setObjectName("Scene"); //para qss


    for (int i = 0; i < 3; ++i) {


        GraphicsTextItem *textItem = new GraphicsTextItem();
        textItem->setPlainText("Texto de ejemplo " + QString::number(i + 1));

        QFont font = textItem->font();
        font.setPointSize(20);
        textItem->setFont(font);


        textItem->setPos(100 + i * 150, 100 + i * 50);
        this->addItem(textItem);
    }




}

Scene::~Scene(){}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    //QGraphicsItem *item = itemAt(event->scenePos(), views().first()->transform());

    QGraphicsScene::mousePressEvent(event);
}







