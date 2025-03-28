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
#include <QIcon>
#include <QFont>


#include "widgets/tabbar.h"

TabBar::TabBar(QWidget *parent):QTabBar(parent){
    this->setObjectName("TabBar"); //para qss


    QFont font;
         font.setPointSize(12);
         font.setBold(false);

         this->setFont(font);

         this->setTabsClosable(true);

         this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor


 }



TabBar::~TabBar(){}
