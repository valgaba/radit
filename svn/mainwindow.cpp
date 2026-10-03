/* This file is part of Coundown
   Copyright 2024, Victor Algaba <victorengine@gmail.com>

   Coundown is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Iradit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Coundown.  If not, see <http://www.gnu.org/licenses/>.
*/


// release/application_res.o] Error 1 puede ser el puto icono radit.ico que no esta ruta




#include "mainwindow.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QMenuBar>
#include <QDesktopServices>
#include <QUrl>


//#include "widgets/TabPlayer.h"
//#include "widgets/container.h"
#include "widgets/TabAuto.h"
#include "widgets/FormAbout.h"
#include "core/config.h"
#include "widgets/fileexplore.h"
//#include "widgets/Player.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){



    mediamanager = new MediaManager(this);
    mediamanager->initialize(); //iniciamos audio



    // Crear la barra de menú
    QMenuBar *menubar =new QMenuBar;
    menubar->setObjectName("menubar"); //para qss


       // Añadir los menús principales (sin submenús por ahora)
       menubar->addMenu("&Archivo");
       menubar->addMenu("&Editar");
       menubar->addMenu("&Herramientas");
      // menubar->addMenu("A&yuda");


       // Crear submenús
       QMenu *ayudaMenu = menubar->addMenu("A&yuda");

       // Añadir acciones al submenú de ayuda
       QAction *accionAyuda = new QAction("Ayuda", this);
       QAction *accionWeb = new QAction("Visita el sitio web", this);

       QAction *accionAcercaDe = new QAction("Acerca de", this);

       ayudaMenu->addAction(accionAyuda);
       ayudaMenu->addAction(accionWeb);
       ayudaMenu->addSeparator();

       ayudaMenu->addAction(accionAcercaDe);



       connect(accionWeb, &QAction::triggered, this, []() {
           QDesktopServices::openUrl(QUrl("http://www.radit.org"));
       });

       connect(accionAcercaDe, &QAction::triggered, this, []() {
           FormAbout *frmabout = new FormAbout;
           frmabout->show();
       });




        this->setMenuBar(menubar);





     //Clipboard &clipboard = Clipboard::instance();


    this->setWindowTitle("Radit[]");



    this->setMinimumSize(200, 200); // si no pongo esto  no se maximiza al pincipio, una puta mierda
   // clipboardlist = new QList<QWidget*>; // crea un objeto grobal para la gestion de un porta papeles

    centralwidget = new QWidget; //widget principal
    this->setCentralWidget(centralwidget);

    splitterprincipal = new QSplitter(centralwidget); // general
    splitterprincipal->setGeometry(QRect(80, 60, 561, 391));
    splitterprincipal->setOrientation(Qt::Vertical);
    splitterprincipal->setHandleWidth(1); //grosor linea separador




    splittertop = new QSplitter(splitterprincipal); //cuadricula parte alta
    splittertop->setOrientation(Qt::Horizontal);
    splittertop->setHandleWidth(1);







    //splitterprincipal->addWidget(splittertop);

    splitterdown = new QSplitter(splitterprincipal); //cuadricula parte baja
    splitterdown->setOrientation(Qt::Horizontal);
    splitterdown->setHandleWidth(1);


    QList<int> sizes;
    sizes << 1 << 2; // permite especificar los tamaños de los widgets secundarios del divisor.
    splitterprincipal->setSizes(sizes);



    QVBoxLayout  *layoutprincipal = new QVBoxLayout(centralwidget); //layout espacio global vertical
    layoutprincipal->setSpacing(0);
    layoutprincipal->setContentsMargins(0, 0, 0, 0);
    layoutprincipal->addWidget(splitterprincipal);


    // ----------------------------------------
    // Crear Players
    // ----------------------------------------

    Player *player1 = new Player;
    Player *player2 = new Player;
    Player *player3 = new Player;
    Player *player4 = new Player;



    player1->setObjectName("Player1");
    player2->setObjectName("Player2");
    player3->setObjectName("Player3");
    player4->setObjectName("Player4");





    // Guardar referencias
    players << player1
            << player2
            << player3
            << player4;



    // Conectar cambios de configuración
    for (Player *player : players) {

        connect(player,
                &Player::configurationChanged,
                this,
                [this]() {
                    Config::saveConfig("config.json",players);
                });
    }




    // ----------------------------------------
    // Añadir Players a la interfaz
    // ----------------------------------------

   // splittertop->addWidget(player1);
    splittertop->addWidget(new FileExplore);
    splittertop->addWidget(new TabAuto);

    splitterdown->addWidget(player2);
    splitterdown->addWidget(player3);
    splitterdown->addWidget(player4);


    // Repartir el ancho: 40 % para FileExplore y 60 % para TabAuto.
    splittertop->setStretchFactor(0, 2);
    splittertop->setStretchFactor(1, 3);
    QList<int> sizesTop;
    sizesTop << 400 << 600;
    splittertop->setSizes(sizesTop);


// cargar configuracion de los player*******************
    Config::loadConfig("config.json",players);


}

MainWindow::~MainWindow(){
   mediamanager->shutdown();//libera recursos

}
