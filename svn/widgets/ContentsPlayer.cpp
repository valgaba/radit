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
#include <QWidgetAction>


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

    selectallAction = new QAction("SelectAll", this);
    cutAction = new QAction("Cut", this);
    copyAction = new QAction("Copy", this);
    pasteAction = new QAction("Paste", this);
    deleteAction = new QAction("Delete", this);
    propertiesAction = new QAction("Properties", this);
    colorAction = new QAction("Color Item", this);


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



    //sub menu para el color
    Menu *colorSubmenu = new Menu(this);

    // Crear un QWidgetAction para contener los botones horizontalmente
    QWidgetAction *colorWidgetAction = new QWidgetAction(colorSubmenu);
    QWidget *colorWidget = new QWidget(colorSubmenu);
    QHBoxLayout *colorLayout = new QHBoxLayout(colorWidget);
    colorLayout->setContentsMargins(0,0,0,0);
    colorLayout->setSpacing(0);

    // Crear botones de color
    struct ColorButton {
        QString name;
        QColor color;
    };
    QVector<ColorButton> colors = {
        {"Red", QColor("#e74c3c")},
        {"Orange", QColor("#e67e22")},
        {"Green", QColor("#7cb342")},
        {"Blue", QColor("#00acc1")},
        {"Cyan", QColor("#3498db")},
        {"Pink", QColor("#d81b60")}
    };

    for (const auto &c : colors) {
        QPushButton *btn = new QPushButton(createColorIcon(c.color), "");
        btn->setFlat(true);
        btn->setToolTip(c.name); // Opcional, para mostrar nombre al pasar el mouse
        colorLayout->addWidget(btn);

        // Conectar clic al color
        connect(btn, &QPushButton::clicked, this, [this, c]() {
            // Obtener items seleccionados
               QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();
               bool hasSelection = false;

               for (AudioItemMaxi* item : items) {
                   if (item->isSelect()) {
                       item->framecolor->setColor(c.color);
                       hasSelection = true;
                   }
               }

               // Si no hay selección, aplicar al item bajo el cursor
               if (!hasSelection) {
                   QWidget *widget = childAt(mousePos);
                   while (widget && !qobject_cast<AudioItemMaxi*>(widget))
                       widget = widget->parentWidget();

                   if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
                       item->framecolor->setColor(c.color);
                   }
               }

               // 🔹 Deseleccionar todos los items
               for (AudioItemMaxi* item : items) {
                   item->setIsSelect(false);
               }


               // 🔹 Cerrar el submenú de colores
               if (menu->isVisible()) {
                   menu->close();
               }
        });
    }

    // Asignar widget al QWidgetAction y añadirlo al submenu
    colorWidgetAction->setDefaultWidget(colorWidget);
    colorSubmenu->addAction(colorWidgetAction);

    // Asignar el submenú al QAction colorAction
    colorAction->setMenu(colorSubmenu);



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
        QList<AudioItemMaxi*> selectedItems;

         // Recoger seleccionados
         QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
         for (AudioItemMaxi* item : std::as_const(items)) {
             if (item->isSelect()) {
                 selectedItems.append(item);
             }
         }

         // Caso 1: hay selección -> mostrar solo el número
         if (!selectedItems.isEmpty()) {

             QString text = QString("¿Seguro que quieres borrar los %1 items seleccionados?")
                                .arg(selectedItems.size());

             QMessageBox::StandardButton reply = QMessageBox::question(
                 this,
                 "Borrar Item",
                 text,
                 QMessageBox::Yes | QMessageBox::No
             );

             if (reply == QMessageBox::No)
                 return;

             for (AudioItemMaxi* item : std::as_const(selectedItems)) {
                 deleteItem(item);
             }

             return;
         }

         // Caso 2: no hay selección, borrar item bajo el cursor -> mostrar nombre
         QWidget *widget = childAt(mousePos);

         while (widget && !qobject_cast<AudioItemMaxi*>(widget)) {
             widget = widget->parentWidget();
         }

         if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {

             QString nombre = item->nameFile();

             QString text = nombre.isEmpty()
                 ? "¿Seguro que quieres borrar este item?"
                 : QString("¿Seguro que quieres borrar \"%1\"?").arg(nombre);

             QMessageBox::StandardButton reply = QMessageBox::question(
                 this,
                 "Borrar Item",
                 text,
                 QMessageBox::Yes | QMessageBox::No
             );

             if (reply == QMessageBox::No)
                 return;

             deleteItem(item);
         }

    });


    connect(selectallAction, &QAction::triggered, this, [this]{

        QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();

        for (AudioItemMaxi* item : std::as_const(items)) {
            item->setIsSelect(true);
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
    menu->addAction(selectallAction);
    menu->addAction(cutAction);
    menu->addAction(copyAction);
    menu->addAction(pasteAction);
    menu->addAction(deleteAction);
    menu->addSeparator(); // Añadir un separador
    menu->addAction(propertiesAction);
    menu->addSeparator(); // Añadir un separador
    menu->addAction(colorAction);


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
           selectallAction->setVisible(false);
           copyAction->setVisible(false);
           cutAction->setVisible(false);
           deleteAction->setVisible(false);
           propertiesAction->setVisible(false);
           pasteAction->setVisible(!this->clipboard.lista.isEmpty());
           menu->exec(mapToGlobal(mousePos));
           return;
       }

       // Mostrar acciones por defecto
       selectallAction->setVisible(true);
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


// color para los menus

QIcon ContentsPlayer::createColorIcon(const QColor &color)
{
    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(2, 2, 12, 12, 3, 3);

    return QIcon(pixmap);
}
