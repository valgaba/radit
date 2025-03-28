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
#include "widgets/AudioItemFile.h"
#include "widgets/FormProperties.h"


ContentsPlayer::ContentsPlayer(QWidget *parent):ContentsBase(parent){



    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada
    this->setObjectName("Contents"); // para el archivo qss
    this->setAcceptDrops(true);

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
    deleteAction->setIcon(QIcon(":/icons/Remove.svg"));
    propertiesAction->setIcon(QIcon(":/icons/properties.svg"));

    /* addAction->setIcon(QIcon(":/icons/Add.svg"));
    cutAction->setIcon(QIcon::fromTheme("edit-cut"));
    copyAction->setIcon(QIcon::fromTheme("edit-copy"));
    pasteAction->setIcon(QIcon::fromTheme("edit-paste"));
    deleteAction->setIcon(QIcon::fromTheme("edit-delete"));
    propertiesAction->setIcon(QIcon(":/icons/properties.svg"));*/



    // Crear el submenú
    Menu *submenu = new Menu(this);
    QAction *addAudiofile = new QAction("Add file", this);
    QAction *subAction2 = new QAction("Elemento 2", this);
    QAction *subAction3 = new QAction("Elemento 3", this);

    // iconos del submenu
    addAudiofile ->setIcon(QIcon(":/icons/net.svg"));


    submenu->addAction(addAudiofile);
    submenu->addAction(subAction2);
    submenu->addAction(subAction3);

    // Agregar el submenú a la opcion añadir
    addAction->setMenu(submenu);


    copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C)); // Establecer atajo de teclado


    //**********************************************

    // aciones **************************************
    connect(addAudiofile, &QAction::triggered,this, [=]{

        this->AddItem(new AudioItemFile);
    });



    connect(propertiesAction, &QAction::triggered,this, [=]{
        QWidget *widget = childAt(mousePos);
        AudioItem *audioitem = qobject_cast<AudioItem*>(widget->parentWidget());


        if(audioitem){
            FormProperties *formproperties= new FormProperties(this);
            formproperties->setItembase(audioitem);
            formproperties->show();
        }

    });


    connect(deleteAction, &QAction::triggered,this, [=]{
        if (QWidget *widget = childAt(mousePos)) {
            if (QWidget *parent = widget->parentWidget()) {
                parent->deleteLater();
            }
        }

    });



    connect(copyAction, &QAction::triggered,this, [=]{

        this->clipboard.lista.clear(); // borramos todo el clipboard
        QWidget *widget = childAt(mousePos);

        if(widget){
            widget->parentWidget()->setProperty("iscut", false); // saber si corta o pega
            this->clipboard.lista.append(widget->parentWidget());
        }

    });


    connect(cutAction, &QAction::triggered,this, [=]{

        this->clipboard.lista.clear(); // borramos todo el clipboard
        QWidget *widget = childAt(mousePos);

        if(widget){
            widget->parentWidget()->setProperty("iscut", true); // para la accion de corte del qwidget
            this->clipboard.lista.append(widget->parentWidget());
        }

    });





    connect(pasteAction, &QAction::triggered, this, [this] {

        if (clipboard.lista.empty()) {
            return;  // Salir temprano si no hay elementos
        }

        bool isCutOperation = !clipboard.lista.isEmpty() &&
                               clipboard.lista.constFirst() &&
                               clipboard.lista.constFirst()->property("iscut").toBool();

        for (QWidget *widget : clipboard.lista) {

            if (auto *itembase = qobject_cast<AudioItem*>(widget)) {
                   this->AddItem(itembase->copy());

                // Eliminar widget si fue cortado
                   if (isCutOperation && widget)
                        widget->deleteLater();

            }
        }

        if (isCutOperation) {
            clipboard.lista.clear();
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


//***********************************


//entra el evento decide si elevento es valido
void ContentsPlayer::dragEnterEvent(QDragEnterEvent *event){
    // Verificar si el arrastre contiene URIs de archivos

        if (event->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QUrl fileUrl = urls.first(); // Tomamos el primer archivo (podrías expandir para múltiples)
            QString filePath = fileUrl.toLocalFile();

                event->acceptProposedAction();
            return;

        }
    }

    // Si no es un archivo de audio válido, ignorar el evento
   // event->ignore();
    event->acceptProposedAction();
}



// miestras se arrastra
void ContentsPlayer::dragMoveEvent(QDragMoveEvent *event){

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
void ContentsPlayer::dropEvent(QDropEvent *event){

    QWidget* source = qobject_cast< QWidget*>(event->source());

    if(source) {

       // if(source->parent()!=this){ // es distinto mismo contents

            // Removemos el widget de su layout original esto puede ser inutil
                if(source->parentWidget() && source->parentWidget()->layout()) {
                    source->parentWidget()->layout()->removeWidget(source);
            }


                this->AddItem(source);  //añadimos el muevo elemento

       // }

        return;
    }




    if (event->mimeData()->hasFormat("text/uri-list")) {
        QList<QUrl> urls = event->mimeData()->urls();

             foreach(QUrl url, urls) {
                   QString filePath = url.toLocalFile();
                   AudioItemFile *audioItem = new AudioItemFile;
                   audioItem->setfilePath(filePath);

                   QFileInfo fileInfo(filePath);
                   audioItem->nombre->setText(fileInfo.completeBaseName());
                   audioItem->setToolTip(filePath);


                   this->AddItem(audioItem);
            }

        event->acceptProposedAction();
    } else {
        event->ignore();
    }

}


//************************************************
void ContentsPlayer::AddItem(QWidget * item){
    layout->addWidget(item);


}



void ContentsPlayer::contextMenuEvent(QContextMenuEvent *event){

    mousePos = event->pos();


    // Obtener el widget en las coordenadas del ratón para las opciones de copy y paste
    QWidget *widget = childAt(event->pos());


    if (widget) {

        copyAction->setVisible(true);
        cutAction->setVisible(true);
        deleteAction->setVisible(true);
        propertiesAction->setVisible(true);
    }else{
        copyAction->setVisible(false);
        cutAction->setVisible(false);
        deleteAction->setVisible(false);
        propertiesAction->setVisible(false);
    }


    if (this->clipboard.lista.isEmpty()) {
        pasteAction->setVisible(false);
    } else {
        pasteAction->setVisible(true);
    }


    menu->popup(mapToGlobal(event->pos()));
    menu->exec(mapToGlobal(event->pos()));


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





