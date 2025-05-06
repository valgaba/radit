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
#include <QPlainTextEdit>
#include <QFile>
#include <QByteArray>

#include "widgets/FormAbout.h"



FormAbout::FormAbout(QWidget *parent):FormProperties(parent){

    this->setObjectName("FormProperties"); //para qss


    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    this->setModal(true); // hacerlo modal

    setWindowTitle("About");  // nombre ventana
    // Set the window icon here
    QIcon windowIcon(":/icons/properties.svg");
    setWindowIcon(windowIcon);



 this->setMinimumSize(300,300); // alto del item
 this->resize(600, 600);         // Tamaño inicial



    texteditabout = new QTextEdit;

    texteditabout->setObjectName("TextEditAbout");
    texteditabout->setReadOnly(true);  // Esto impide la edición



    // Cargar el archivo HTML
    QFile archivo("about.html");
    if (!archivo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QByteArray datos = archivo.readAll();
    archivo.close();

    QString contenido = QString::fromUtf8(datos);
    texteditabout->setHtml(contenido);





    btnaccept = new Button;
    btnaccept->setProperty("class", "Buttonpropertie");
    btnaccept->setText("Accept");
    connect(btnaccept, &QPushButton::clicked, this, &QDialog::reject); //salir



   this->addTopWidget(texteditabout,0,0);
    this->addDownWidget(btnaccept);




}

FormAbout::~FormAbout(){}

