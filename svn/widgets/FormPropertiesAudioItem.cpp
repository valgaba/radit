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



#include "widgets/FormPropertiesAudioItem.h"
#include "widgets/ContentsPlayer.h"





FormPropertiesAudioItem::FormPropertiesAudioItem(QWidget *parent):FormProperties(parent){

   this->setObjectName("FormProperties"); //para qss


   setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
   //this->setModal(true); // hacerlo modal

   setWindowTitle("Properties");  // nombre ventana
   // Set the window icon here
   QIcon windowIcon(":/icons/properties.svg");
   setWindowIcon(windowIcon);



this->setMinimumSize(300,300); // alto del item
this->resize(600, 600);         // Tamaño inicial



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


   grupcontainer =new Container; // para los grupos
 // grupcontainer->setProperty("class", "Contentspropertie");
  grupcontainer->widget()->setProperty("class", "Contentspropertie");



   ContentsPlayer *contentsplayer = dynamic_cast<ContentsPlayer*>(grupcontainer->widget());
   contentsplayer->setObjectName("grups"); //para qss


   gridlayout->addWidget(namefile,1,0 );
   gridlayout->addWidget(textfile,1,1 );
   gridlayout->addWidget(btnfile,1,2 );

   gridlayout->addWidget(grupcontainer,3,1 );

   layout->addWidget(container);




   // =============== parte baja para la botonera



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

   // frame->setLayout(downlayout);

    layout->addWidget(frame);




}

FormPropertiesAudioItem::~FormPropertiesAudioItem(){}


void FormPropertiesAudioItem::setItembase(AudioItem *itembase){

   this->itembase=itembase;


}

AudioItem* FormPropertiesAudioItem::getItembase(){

   return (this->itembase);

}









