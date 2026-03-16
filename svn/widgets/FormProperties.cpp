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
#include <QSpacerItem>
#include <QRegularExpressionValidator>



#include "widgets/FormProperties.h"



FormProperties::FormProperties(QWidget *parent):QDialog(parent){


    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop); // pone los item en la parte alta
    layout->setSpacing(0); // espacios entre  item dentro del contenedor
    this->setLayout(layout);

    container = new Container;
    contents = new QWidget;  // el contenido por defecto del container no nos sirve
    contents->setProperty("class", "Contentspropertie");


     toplayout = new QGridLayout; //layout para en contenido
     toplayout->setAlignment(Qt::AlignTop);
     contents->setLayout(toplayout);

     container->setWidget(contents); // añadimos en contenido al container




    // =============== parte baja para la botonera

     frame = new Frame;
     QSpacerItem * spacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

     downlayout = new QHBoxLayout;
     downlayout->setContentsMargins(1, 1, 1, 1);
     downlayout->addItem(spacer);
     frame->setLayout(downlayout);




      //añadimos el principal
      layout->addWidget(container); //parte alta
      layout->addWidget(frame);     //parte baja


}

FormProperties::~FormProperties(){}



//añadimos widget a las zonas****************************
void FormProperties::addTopWidget(QWidget *widget, int row, int column){
     toplayout->addWidget(widget,row,column);
}


void FormProperties::addDownWidget(QWidget *widget){
    downlayout->addWidget(widget);
}








