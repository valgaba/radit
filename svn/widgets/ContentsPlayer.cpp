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
//#include "widgets/AudioItemFileMini.h"
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
        // Buscar todos los AudioItemMaxi seleccionados
          QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
          bool hasSelection = false;

          for (AudioItemMaxi* item : std::as_const(items)) {
              if (item->isSelect()) {
                  item->deleteLater();
                  hasSelection = true;
              }
          }

          // Si no hay seleccionados, borrar el item bajo el cursor
          if (!hasSelection) {
              QWidget *widget = childAt(mousePos);

              while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
                  widget = widget->parentWidget();
              }

              if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
                  item->deleteLater();
              }
          }

    });



    connect(copyAction, &QAction::triggered,this, [=]{

        clipboard.lista.clear();

            QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();
            bool hasSelection = false;

            for (AudioItemMaxi *item : std::as_const(items)) {
                if (item->isSelect()) {
                    item->setProperty("iscut", false);
                    clipboard.lista.append(item);
                    hasSelection = true;
                }


            }

            // Si no hay selección usar el item bajo el cursor
            if (!hasSelection) {

                QWidget *widget = childAt(mousePos);

                while (widget && !qobject_cast<AudioItemMaxi*>(widget))
                    widget = widget->parentWidget();

                if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
                    item->setProperty("iscut", false);
                    clipboard.lista.append(item);
                }
            }

    });


    connect(cutAction, &QAction::triggered,this, [=]{

        clipboard.lista.clear();

            QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();
            bool hasSelection = false;

            for (AudioItemMaxi *item : std::as_const(items)) {
                if (item->isSelect()) {
                    item->setProperty("iscut", true);
                    clipboard.lista.append(item);
                    hasSelection = true;
                }


            }

            if (!hasSelection) {

                QWidget *widget = childAt(mousePos);

                while (widget && !qobject_cast<AudioItemMaxi*>(widget))
                    widget = widget->parentWidget();

                if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
                    item->setProperty("iscut", true);
                    clipboard.lista.append(item);
                }
            }
    });





    connect(pasteAction, &QAction::triggered, this, [this] {

        if (clipboard.lista.empty())
                return;

            bool isCutOperation = clipboard.lista.constFirst()->property("iscut").toBool();

            for (auto it = clipboard.lista.begin(); it != clipboard.lista.end(); ) {

                if (auto *itembase = qobject_cast<AudioItemMaxi*>(*it)) {

                    AudioItemMaxi *newItem = itembase->copy(this);
                    createItem(newItem);

                    // 🔹 quitar selección del nuevo item pegado
                    newItem->setIsSelect(false);

                    if (isCutOperation) {
                        itembase->deleteLater();
                        it = clipboard.lista.erase(it);
                    } else {
                        ++it;
                    }

                } else {
                    ++it;
                }
            }

            // limpiar selección de los originales
            for (auto it = clipboard.lista.begin(); it != clipboard.lista.end(); ++it) {
                if (auto *item = qobject_cast<AudioItemMaxi*>(*it)) {
                    item->setIsSelect(false);
                }
            }

    });


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




void ContentsPlayer::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-audioitems")) {

            QByteArray data = event->mimeData()->data("application/x-audioitems");
            QDataStream stream(&data, QIODevice::ReadOnly);

            QList<AudioItem*> movedItems;

            while (!stream.atEnd()) {

                quintptr ptr;
                stream >> ptr;

                AudioItem *item = reinterpret_cast<AudioItem*>(ptr);
                if (!item)
                    continue;

                if (item->parentWidget() && item->parentWidget()->layout())
                    item->parentWidget()->layout()->removeWidget(item);

                layout->addWidget(item);
                item->setParent(this);

                movedItems.append(item);
            }

            // Desmarcar los items seleccionados después del drag
            for (AudioItem* item : movedItems) {
                item->setIsSelect(false);
            }

            event->acceptProposedAction();
            return;
        }

        // Si no es drag interno, usar el comportamiento del padre
        ContentsBase::dropEvent(event);
}



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

