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


#include "widgets/TabPlayer.h"
#include "widgets/container.h"
#include "widgets/TabAuto.h"
#include "widgets/FormAbout.h"
#include "bass.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent){


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

    this->IniAudio();

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




    TabPlayer *tab = new TabPlayer;

    for (int i = 0; i < 4; ++i) {
        Container *container = new Container;
        tab->addTab(container, "lista " + QString::number(i + 1));
    }


   TabPlayer *tab2 = new TabPlayer;

    for (int i = 0; i < 4; ++i) {
       // Container *container = new Container;
        Container *container = new Container;
        tab2->addTab(container, "lista " + QString::number(i + 1));
    }

    TabPlayer *tab3 = new TabPlayer;

    for (int i = 0; i < 4; ++i) {
        Container *container = new Container;
        tab3->addTab(container, "lista " + QString::number(i + 1));
    }



    TabPlayer *tab4 = new TabPlayer;

    for (int i = 0; i < 4; ++i) {
        Container *container = new Container;
        tab4->addTab(container, "lista " + QString::number(i + 1));
    }



    splitterdown->addWidget(tab4);
    splitterdown->addWidget(tab2);
    splitterdown->addWidget(tab3);



    splittertop->addWidget(tab);
    splittertop->addWidget(new TabAuto);



    // Ajustar tamaños equitativos
    QList<int> sizesTop;
    sizesTop << 1 << 1;  // Asigna el mismo peso a ambos widgets
    splittertop->setSizes(sizesTop);




}

MainWindow::~MainWindow(){


}

//------------------------------------------
void MainWindow::IniAudio(){

    BASS_DEVICEINFO i;
    int device = -1;  // default en windows

    //bass configuration
    BASS_SetConfig(BASS_CONFIG_BUFFER, 5000 );
    // BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 10);
    BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST,1); // enable playlist processing
    //BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT,false);

    BASS_Init(device, 44100,0, 0, NULL); // abrimos dispositivo por defecto


    // plugin

    //BASS_PluginFree(0);
    QString basePath = QCoreApplication::applicationDirPath() + "/bassplugin";

    #ifdef _WIN32


    QStringList plugins = {
        "bass_aac.dll",
        "bassflac.dll",
        "basswma.dll"
    };

    for (const QString &plugin : plugins)
    {
        QString fullPath = basePath + "/" + plugin;
        HPLUGIN handle = BASS_PluginLoad(fullPath.toUtf8().constData(), 0);

        if (!handle)
        {
            qDebug() << "Error cargando plugin:" << fullPath
                     << "Error code:" << BASS_ErrorGetCode();
        }
        else
        {
            qDebug() << "Plugin cargado:" << fullPath;
        }
    }

    #endif


    #ifdef Q_OS_UNIX
        BASS_PluginLoad(Path.toLatin1() + "/Plugin/libbass_aac.so",0);
        BASS_PluginLoad(Path.toLatin1() + "/Plugin/libbassflac.so",0);
         device=1; // el defaul en linux
    #endif



     //****************** Abrimmos dispositivos de audio *****************************
        for (int c=0;BASS_GetDeviceInfo(c,&i);c++)// device 1 = el primer dispositivo
        {
           if (i.flags&BASS_DEVICE_ENABLED){

                // BASS_Init(c, 44100,0, 0, NULL);

              }
        }


}


