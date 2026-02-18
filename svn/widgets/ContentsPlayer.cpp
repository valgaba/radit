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


#include <QDrag>
#include <QDragEnterEvent>
#include <QDebug>
#include <QLayoutItem>
#include <QMimeData>
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QIcon>
#include <QUrl>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>


#include "widgets/ContentsPlayer.h"
#include "widgets/AudioItem.h"
#include "widgets/AudioItemFileMini.h"
#include "widgets/AudioItemFileMaxi.h"
#include "widgets/FormPropertiesAudioItem.h"
#include "core/io.h"


ContentsPlayer::ContentsPlayer(QWidget *parent):ContentsBase(parent){



    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada
    this->setObjectName("Contents"); // para el archivo qss
   // this->setAcceptDrops(true);

    this->isCut=false;





    // creamos menu contectual

    menu = new Menu(this);
    menu->setFixedWidth(200); // Establecer anchura del menú en píxeles

    QAction *addAction = new QAction("Añadir  ", this);
    cutAction = new QAction("Cortar", this);
    copyAction = new QAction("Copiar", this);
    pasteAction = new QAction("Pegar", this);
    deleteAction = new QAction("Delete", this);
    propertiesAction = new QAction("Properties", this);


    //iconos del menu
    addAction->setIcon(QIcon(":/icons/Add.svg"));
    cutAction->setIcon(QIcon(":/icons/ActionCut.svg"));
    copyAction->setIcon(QIcon(":/icons/ActionCopy.svg"));
    pasteAction->setIcon(QIcon(":/icons/ActionPaste.svg"));
    deleteAction->setIcon(QIcon(":/icons/Deletemenu.svg"));
    propertiesAction->setIcon(QIcon(":/icons/properties.svg"));

    // Crear el submenú
    Menu *submenu = new Menu(this);
    QAction *addAudiofile = new QAction("Audio File", this);
    QAction *addAudioFolder = new QAction("Audio Folder", this);
    QAction *addNeturl = new QAction("Net Url", this);
    QAction *addGroup = new QAction("Group", this);

    // iconos del submenu
    addAudiofile ->setIcon(QIcon(":/icons/audiofile.svg"));
    addAudioFolder ->setIcon(QIcon(":/icons/folder.svg"));
    addNeturl ->setIcon(QIcon(":/icons/net.svg"));


    submenu->addAction(addAudiofile);
    submenu->addAction(addAudioFolder);
    submenu->addAction(addNeturl);
    submenu->addAction(addGroup);

    // Agregar el submenú a la opcion añadir
    addAction->setMenu(submenu);


    copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C)); // Establecer atajo de teclado


    //**********************************************

    // aciones **************************************
    connect(addAudiofile, &QAction::triggered,this, [this]{

        createItem(new AudioItemFileMaxi(this));

      //  layout->addWidget(new AudioItemFileMaxi);
      /*  AudioItemFileMaxi *item = new AudioItemFileMaxi;

        layout->addWidget(item);

        connect(item, &AudioItemMaxi::requestDelete,  //señal de borrado
                this, [this](AudioItemMaxi* item)
        {
            layout->removeWidget(item);  // quitar del layout
            item->deleteLater();         // borrar seguro
        });*/



    });



    connect(addGroup, &QAction::triggered,this, [this]{

        Io *io = new Io;

        io->saveWidgetsToJson(this->layout,"salidajson.txt");

    });

    connect(addNeturl, &QAction::triggered,this, [this]{

        Io *io = new Io;

        io->loadWidgetsFromJson(this->layout,"salidajson.txt");

    });





    connect(propertiesAction, &QAction::triggered,this, [this]{
        QWidget *widget = childAt(mousePos);
        if (!widget) return;

        AudioItem *audioitem = qobject_cast<AudioItem*>(widget->parentWidget());

        if(audioitem){
            FormPropertiesAudioItem *formpropertiesaudioitem= new FormPropertiesAudioItem(this);
            formpropertiesaudioitem->setItembase(audioitem);
            formpropertiesaudioitem->show();
        }

    });


    connect(deleteAction, &QAction::triggered,this, [this]{
        QWidget *widget = childAt(mousePos);

          while (widget && !qobject_cast<AudioItem*>(widget)) {
              widget = widget->parentWidget();
          }

          if (widget) {
              widget->deleteLater();
          }
    });



    connect(copyAction, &QAction::triggered,this, [=]{

        this->clipboard.lista.clear(); // borramos todo el clipboard
        QWidget *widget = childAt(mousePos);

        while (widget && !qobject_cast<AudioItem*>(widget)) {
               widget = widget->parentWidget();

           }


        if(widget){
            widget->setProperty("iscut", false); // saber si corta o pega
            this->clipboard.lista.append(widget);
        }

    });


    connect(cutAction, &QAction::triggered,this, [=]{

        this->clipboard.lista.clear(); // borramos todo el clipboard
        QWidget *widget = childAt(mousePos);

        while (widget && !qobject_cast<AudioItem*>(widget)) {
               widget = widget->parentWidget();

           }


        if(widget){
            widget->setProperty("iscut", true); // para la accion de corte del qwidget
            this->clipboard.lista.append(widget);
        }

    });





    connect(pasteAction, &QAction::triggered, this, [this] {

        if (clipboard.lista.empty()) return;  // Salir temprano si no hay elementos

         bool isCutOperation = !clipboard.lista.isEmpty() &&
                               clipboard.lista.constFirst() &&
                               clipboard.lista.constFirst()->property("iscut").toBool();

        for (auto it = clipboard.lista.begin(); it != clipboard.lista.end();) {
            if (auto *itembase = qobject_cast<AudioItemMaxi*>(*it)) { //polimorfica pega todos los hijos de audioitem
                AudioItemMaxi* newItem = itembase->copy(this);
                 createItem(newItem);

               /* AudioItem* newItem = itembase->copy(this);  // crear copia
                layout->addWidget(newItem);


                if (auto maxiItem = qobject_cast<AudioItemMaxi*>(newItem)) {

                    connect(maxiItem, &AudioItemMaxi::requestDelete,
                            this, [this](AudioItemMaxi* item)
                    {
                        layout->removeWidget(item);
                        item->deleteLater();
                    });
                }*/

                if (isCutOperation && *it) {
                    (*it)->deleteLater();
                    it = clipboard.lista.erase(it);  // Eliminar de la lista y avanzar el iterador
                } else {
                    ++it;
                }
            } else {
                ++it;
            }
        }
    });




    // connect(copyAction, &QAction::triggered, this, &MyWidget::copyActionTriggered);
    // connect(cutAction, &QAction::triggered, this, &MyWidget::cutActionTriggered);
    //  connect(pasteAction, &QAction::triggered, this, &MyWidget::pasteActionTriggered);


    menu->addAction(addAction);
    menu->addSeparator(); // Añadir un separador
    menu->addAction(cutAction);
    menu->addAction(copyAction);
    menu->addAction(pasteAction);
    menu->addAction(deleteAction);
    menu->addSeparator(); // Añadir un separador
    menu->addAction(propertiesAction);


}



ContentsPlayer::~ContentsPlayer(){}




void ContentsPlayer::contextMenuEvent(QContextMenuEvent *event){
    mousePos = event->pos();

       QWidget *widget = childAt(mousePos);

       // Si no hay widget, oculta todas las opciones y retorna
       if (!widget) {
           copyAction->setVisible(false);
           cutAction->setVisible(false);
           deleteAction->setVisible(false);
           propertiesAction->setVisible(false);
           pasteAction->setVisible(!this->clipboard.lista.isEmpty());
           menu->exec(mapToGlobal(mousePos));
           return;
       }

       // Mostrar acciones por defecto
       copyAction->setVisible(true);
       cutAction->setVisible(true);
       deleteAction->setVisible(true);
       propertiesAction->setVisible(true);

       // Verifica si el widget tiene padres válidos antes de acceder a ellos
       QWidget *parent1 = widget->parentWidget();
       QWidget *parent2 = parent1 ? parent1->parentWidget() : nullptr;

       if (parent2 && parent2->objectName() == "grups") {
             propertiesAction->setVisible(false);
       }

       // Determinar visibilidad de "Paste"
       pasteAction->setVisible(!this->clipboard.lista.isEmpty());

       // Mostrar menú contextual
       menu->exec(mapToGlobal(mousePos));

}


AudioItemMaxi* ContentsPlayer::createItem(AudioItemMaxi* item){

    layout->addWidget(item);

    connect(item, &AudioItemMaxi::requestDelete,  //señal de borrado
            this, [this](AudioItemMaxi* item)
    {
        layout->removeWidget(item);  // quitar del layout
        item->deleteLater();         // borrar seguro
    });

      return item;



}

////////////////esto es para el qss **********+

/*Como se dice en la referencia de hojas de estilo de Qt,
aplicar estilos CSS a widgets personalizados heredados de QWidget
requiere volver a implementar paintEvent() de esa manera:
En un Qframe no haria falta
*/

void ContentsPlayer::paintEvent(QPaintEvent *){

    /* QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);*/

}





