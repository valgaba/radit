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
#include <QMessageBox>

//#include <QtConcurrent/QtConcurrentMap>
//#include <QFuture>
//#include <QFutureWatcher>

#include "widgets/contentsbase.h"
//#include "widgets/AudioItemFileMini.h"
#include "widgets/AudioItemFilemaxi.h"
//#include "bass.h"

ContentsBase::ContentsBase(QWidget *parent):QWidget(parent){
  // setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada

    this->setObjectName("Contents"); // para el archivo qss
    this->setAcceptDrops(true);



    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop); // pone los item en la parte alta
    layout->setSpacing(5); // espacios entre  item dentro del contenedor
    this->setLayout(layout);

    mediamanager = new MediaManager(this);


}



ContentsBase::~ContentsBase(){}

//***********************************


//entra el evento decide si elevento es valido
void ContentsBase::dragEnterEvent(QDragEnterEvent *event){
    // Verificar si el arrastre contiene URIs de archivos

    //  Drag interno (widgets)
       if (qobject_cast<QWidget*>(event->source())) {
           event->acceptProposedAction();
           return;
       }

       //  Archivos externos
       if (event->mimeData()->hasUrls()) {

           const QList<QUrl> urls = event->mimeData()->urls();

           for (const QUrl &url : urls) {
               if (url.isLocalFile()) {
                   event->acceptProposedAction();
                   return;
               }
           }
       }

       // Si no cumple ninguna condición
       event->ignore();
}



// miestras se arrastra
void ContentsBase::dragMoveEvent(QDragMoveEvent *event){

    event->acceptProposedAction();

}


// suelta evento
void ContentsBase::dropEvent(QDropEvent *event){

  //////////// viene del sistema de archivos
   if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
            foreach(QUrl url, urls) {

               QString filePath = url.toLocalFile();
               double duration = mediamanager->getDurationSecond(filePath);


                   if (duration <= 0.0) {
                       qDebug() << "Archivo inválido:" << filePath;
                       continue; // saltar este archivo
                   }

                   AudioItemFileMaxi *audioItem = new AudioItemFileMaxi(this);
                   QFileInfo fileInfo(filePath);

                   audioItem->setFilePath(filePath);
                   audioItem->setSecond(duration);
                   audioItem->setNameFile(fileInfo.completeBaseName());
                   audioItem->setTiempoFile(duration);
                   audioItem->setToolTip(filePath);

                   createItem(audioItem );
            }

        event->acceptProposedAction();
    } else {
        event->ignore();
    }


}



AudioItemMaxi* ContentsBase::createItem(AudioItemMaxi* item){

    layout->addWidget(item);

    connect(item, &AudioItemMaxi::requestDelete, //viene de pulsar boton de borrado de audioitemMaxi
            this, [this](AudioItemMaxi* item){

      //  if(item->isPlaying())
        //    return;

        QString nombre = item->nameFile();

          QMessageBox::StandardButton reply = QMessageBox::question(
              this,
              "Borrar Item",
              QString("¿Seguro que quieres borrar \"%1\"?").arg(nombre),
              QMessageBox::Yes | QMessageBox::No
          );

          if (reply == QMessageBox::Yes) {
              deleteItem(item);
          }

    });

    //borrado para el purge
   connect(item, &AudioItemMaxi::requestAutoDelete,
           this, [this](AudioItemMaxi* item){
             //if(item->isPlayNext()) // si esta en playnext no borra el item
                // return;

             deleteItem(item);
   });

    //  NUEVO: conectar play grande del item
      connect(item, &AudioItemMaxi::requestPlay,
              this, [this](AudioItemMaxi* item) {

          if (Player* player = findPlayer()) {
              player->playItem(item);
          }

      });

    return item;
}


void ContentsBase::deleteItem(AudioItemMaxi* item){

    if (!item) return;


        //  Buscar el player REAL del item
        Player* itemPlayer = nullptr;
        QWidget* w = item;

        while (w) {
            if (Player* p = qobject_cast<Player*>(w)) {
                itemPlayer = p;
                break;
            }
            w = w->parentWidget();
        }

        if (itemPlayer && item->isPlaying()) {
            itemPlayer->stopMain();
        }

        if (item->parentWidget() && item->parentWidget()->layout()) {
            item->parentWidget()->layout()->removeWidget(item);
        }

        item->deleteLater();
}

//*****************************************
Player* ContentsBase::findPlayer() const{

    QWidget* w = const_cast<ContentsBase*>(this);

    while (w) {
        if (Player* player = qobject_cast<Player*>(w)) {
            return player;
        }
        w = w->parentWidget();
    }

    return nullptr;
}

AudioItemMaxi* ContentsBase::findNextPlayItem(AudioItemMaxi* current)
{
    if (!current || !layout)
            return nullptr;

        int index = layout->indexOf(current);

        //  1. Buscar hacia abajo
        for (int i = index; i < layout->count(); ++i) {
            QWidget* w = layout->itemAt(i)->widget();
            if (auto *item = qobject_cast<AudioItemMaxi*>(w)) {
                if (item->isPlayNext())
                    return item;
            }
        }

        //  2. Si no encuentra, buscar desde arriba
        for (int i = 0; i < index; ++i) {
            QWidget* w = layout->itemAt(i)->widget();
            if (auto *item = qobject_cast<AudioItemMaxi*>(w)) {
                if (item->isPlayNext())
                    return item;
            }
        }

        return nullptr;
}
