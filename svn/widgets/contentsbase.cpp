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

//#include <QtConcurrent/QtConcurrentMap>
//#include <QFuture>
//#include <QFutureWatcher>

#include "widgets/contentsbase.h"
//#include "widgets/AudioItemFileMini.h"
#include "widgets/AudioItemFilemaxi.h"
#include "bass.h"

ContentsBase::ContentsBase(QWidget *parent):QWidget(parent){
  // setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada

    this->setObjectName("Contents"); // para el archivo qss
    this->setAcceptDrops(true);



    layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop); // pone los item en la parte alta
    layout->setSpacing(10); // espacios entre  item dentro del contenedor
    this->setLayout(layout);

    mediamanager = new MediaManager;


}



ContentsBase::~ContentsBase(){}

//***********************************


//entra el evento decide si elevento es valido
void ContentsBase::dragEnterEvent(QDragEnterEvent *event){
    // Verificar si el arrastre contiene URIs de archivos

        if (event->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QUrl fileUrl = urls.first(); // Tomamos el primer archivo (podrías expandir para múltiples)
          //  QString filePath = fileUrl.toLocalFile();

                event->acceptProposedAction();
            return;

        }
    }

    // Si no es un archivo de audio válido, ignorar el evento
   // event->ignore();
    event->acceptProposedAction();
}



// miestras se arrastra
void ContentsBase::dragMoveEvent(QDragMoveEvent *event){

    // Aceptar el movimiento solo si es un archivo de audio válido
    if (event->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QUrl fileUrl = urls.first();
            QString filePath = fileUrl.toLocalFile();

               event->acceptProposedAction();
            return;

        }
    }
  //  event->ignore();
    event->acceptProposedAction();

}

// suelta evento
void ContentsBase::dropEvent(QDropEvent *event){

    QWidget* source = qobject_cast< QWidget*>(event->source());

    if(source) {

            if(source->parentWidget() && source->parentWidget()->layout()) {
                 source->parentWidget()->layout()->removeWidget(source);
              }

            layout->addWidget(source);
            return;
    }

  //////////// viene del sistema de archivos
   if (event->mimeData()->hasFormat("text/uri-list")) {
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
                   audioItem->setToolTip(filePath);
                   audioItem->setNameFile(fileInfo.completeBaseName());
                   audioItem->setSecond(duration);
                   audioItem->setTiempoFile(duration);



                   createItem(audioItem );
            }

        event->acceptProposedAction();
    } else {
        event->ignore();
    }




}



AudioItemMaxi* ContentsBase::createItem(AudioItemMaxi* item)
{
    layout->addWidget(item);

    connect(item, &AudioItemMaxi::requestDelete,
            this, [this](AudioItemMaxi* item)
    {
        layout->removeWidget(item);
        item->deleteLater();
    });

    return item;
}



QString ContentsBase::formatTimeHhMmSsDd(double duration){

    if (duration < 0)
        return "00:00:00.00";

    int hours = static_cast<int>(duration) / 3600;
    int minutes = (static_cast<int>(duration) % 3600) / 60;
    int seconds = static_cast<int>(duration) % 60;

    // Centésimas (2 decimales)
    int centiseconds = static_cast<int>((duration - static_cast<int>(duration)) * 100);

    return QString("%1:%2:%3.%4")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
            .arg(centiseconds, 2, 10, QChar('0'));
}


