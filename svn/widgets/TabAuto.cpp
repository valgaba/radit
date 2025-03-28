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



#include "widgets/TabAuto.h"
#include "widgets/tabbar.h"
#include "widgets/container.h"
#include "widgets/ContentsPlayer.h"

TabAuto::TabAuto(QWidget *parent):Tab(parent){
    this->setObjectName("TabAuto"); //para qss
    //  this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor

    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada




    TabBar *tabbar=new TabBar(this);
    tabbar->setTabsClosable(false); //evita que se cierre las pestañas

    this->setTabBar(tabbar);

    QStringList dias = {"Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado", "Domingo"};



    for (const QString &dia : dias) {
        Container *container = new Container;
        container->setWidget(new ContentsPlayer);
        this->addTab(container, dia);
    }




}



TabAuto::~TabAuto(){}



