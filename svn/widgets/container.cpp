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


#include <QPushButton>

#include "widgets/container.h"
#include "widgets/scrollbar.h"
#include "widgets/ContentsPlayer.h"




Container::Container(QWidget *parent):QScrollArea(parent){

     this->setObjectName("Container"); // para el archivo qss
     this->setWidgetResizable(true); //hace que ocupe todo el dia
     this->setVerticalScrollBar(new ScrollBar); // esta formula de añadir es nueva
     this->setHorizontalScrollBar(new ScrollBar);


    this->setContextMenuPolicy(Qt::NoContextMenu);

    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);




    //this->setWidget(contents);


 // contents = new Contents;
   this->setWidget(new ContentsPlayer); // contenido por defecto


}



Container::~Container(){}






