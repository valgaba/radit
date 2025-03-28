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
#include <QDragEnterEvent>


#include "widgets/TabPlayer.h"
#include "widgets/tabbar.h"

TabPlayer::TabPlayer(QWidget *parent):Tab(parent){
    this->setObjectName("TabPlayer"); //para qss
    //  this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor

    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada




    TabBar *tabbar=new TabBar(this);

    connect(tabbar, &TabBar::tabCloseRequested, [=](int index){
        QWidget* widgetToRemove = this->widget(index);
        delete widgetToRemove;
    });


    this->setTabBar(tabbar);


    menu = new Menu(this);
    menu->setFixedWidth(200); // Establecer anchura del menú en píxeles

    QAction *addAction = new QAction("Añadir pestaña", this);
    QAction *removeAction = new QAction("Eliminar pestaña", this);

    addAction->setIcon(QIcon(":/icons/Add.svg"));
    removeAction->setIcon(QIcon(":/icons/Remove.svg"));




    addAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C)); // Establecer atajo de teclado



    // connect(copyAction, &QAction::triggered, this, &MyWidget::copyActionTriggered);
    // connect(cutAction, &QAction::triggered, this, &MyWidget::cutActionTriggered);
    //  connect(pasteAction, &QAction::triggered, this, &MyWidget::pasteActionTriggered);


    menu->addAction(addAction);
    menu->addAction(removeAction);




}



TabPlayer::~TabPlayer(){}

void TabPlayer::contextMenuEvent(QContextMenuEvent *event)
{

    menu->popup(mapToGlobal(event->pos()));
    menu->exec(mapToGlobal(event->pos()));

}

