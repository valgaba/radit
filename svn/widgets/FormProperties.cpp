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
#include "widgets/frame.h"





FormProperties::FormProperties(QWidget *parent):QDialog(parent){

    this->setObjectName("FormProperties"); //para qss


    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    //this->setModal(true); // hacerlo modal

    setWindowTitle("Properties");  // nombre ventana
    // Set the window icon here
    QIcon windowIcon(":/icons/properties.svg");
    setWindowIcon(windowIcon);



 this->setMinimumSize(300,300); // alto del item
 this->resize(600, 400);         // Tamaño inicial

    layout = new QVBoxLayout;
    layout->setContentsMargins(10, 20, 10, 20);
    layout->setAlignment(Qt::AlignTop); // pone los item en la parte alta
    layout->setSpacing(10); // espacios entre  item dentro del contenedor
    this->setLayout(layout);

    container = new Container;

    contents = new QWidget;  // el contenido por defecto del container no nos sirve

    contents->setObjectName("Contents"); // para el archivo qss
    gridlayout = new QGridLayout;
    gridlayout->setAlignment(Qt::AlignTop);
    contents->setLayout(gridlayout);

    container->setWidget(contents); // añadimos en contenido al container



    //************************************************

    namehour = new QLabel("Hora de entrada");
    namehour->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    namehour->setProperty("class", "Labelpropertie");


    texthour = new QLineEdit("13:00:00");
    texthour->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    texthour->setProperty("class", "LineEditpropertie");


    //Definir una expresión regular para el formato HH:MM:SS (24 horas)
    QRegularExpression regex("^([01]?[0-9]|2[0-3]):[0-5][0-9]:[0-5][0-9]$");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regex, texthour);

    // Aplicar el validador al QLineEdit
    texthour->setValidator(validator);



    gridlayout->addWidget(namehour,0,0 );
  //  gridlayout->addWidget(textfile,0,1 );
    gridlayout->addWidget(texthour,0,1 );



    // file ***********************************
    namefile = new QLabel("Local file");
    namefile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    namefile->setProperty("class", "Labelpropertie");



    textfile = new QLineEdit("C:/Users/victo/Desktop/musica/FIESTA 80/LISTA");
    textfile->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    textfile->setProperty("class", "LineEditpropertie");



    btnfile = new Button;
    btnfile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btnfile->setProperty("class", "Buttonpropertie");
    btnfile->setText("File");


    grupcontainer =new Container;


    gridlayout->addWidget(namefile,1,0 );
    gridlayout->addWidget(textfile,1,1 );
    gridlayout->addWidget(btnfile,1,2 );

    gridlayout->addWidget(grupcontainer,3,1 );

    layout->addWidget(container);




    // =============== parte baja para la botonera

     Frame *frame = new Frame;
     QHBoxLayout *downlayout = new QHBoxLayout;
     downlayout->setContentsMargins(10, 0, 10, 0);


     btncancel = new Button;
     btncancel->setProperty("class", "Buttonpropertie");
     btncancel->setText("Cancel");

     btnaccept = new Button;
     btnaccept->setProperty("class", "Buttonpropertie");

     btnaccept->setText("Accept");
     QSpacerItem * spacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

     downlayout->addItem(spacer);
     downlayout->addWidget(btncancel);
     downlayout->addWidget(btnaccept);

     frame->setLayout(downlayout);

     layout->addWidget(frame);




}

FormProperties::~FormProperties(){}


void FormProperties::setItembase(AudioItem *itembase){

    this->itembase=itembase;

    QString hora = QString("%1:%2:%3")
                       .arg(itembase->gethour(), 2, 10, QChar('0'))
                       .arg(itembase->getminute(), 2, 10, QChar('0'))
                       .arg(itembase->getsecond(), 2, 10, QChar('0'));

    this->texthour->setText(hora);

}

AudioItem* FormProperties::getItembase(){

    return (this->itembase);

}









