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


ContentsPlayer::ContentsPlayer(QWidget *parent):ContentsBase(parent){

    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada
    this->setObjectName("Contents"); // para el archivo qss
    this->isCut=false;


 contentsMenu = new ContentsMenu(this);

}



ContentsPlayer::~ContentsPlayer(){}




void ContentsPlayer::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-audioitems")) {

            QByteArray data = event->mimeData()->data("application/x-audioitems");
            QDataStream stream(&data, QIODevice::ReadOnly);

            QList<AudioItem*> movedItems;
            QList<AudioItemMaxi*> itemsToDeleteFromSource; // <-- borrar después

            while (!stream.atEnd()) {

                quintptr ptr;
                stream >> ptr;

                AudioItem *itemBase = reinterpret_cast<AudioItem*>(ptr);
                if (!itemBase)
                    continue;

                // Ver si viene del mismo ContentsPlayer o de otro
                ContentsPlayer* sourceContents = qobject_cast<ContentsPlayer*>(itemBase->parentWidget());

                if (sourceContents == this) {
                    // MISMO CONTENTSPLAYER => mover
                    if (itemBase->parentWidget() && itemBase->parentWidget()->layout())
                        itemBase->parentWidget()->layout()->removeWidget(itemBase);

                    layout->addWidget(itemBase);
                    itemBase->setParent(this);

                    itemBase->setIsSelect(false);
                    movedItems.append(itemBase);
                }
                else {
                    // OTRO CONTENTSPLAYER => copiar y luego borrar original
                    AudioItemMaxi* itemMaxi = qobject_cast<AudioItemMaxi*>(itemBase);
                    if (!itemMaxi)
                        continue;

                    AudioItemMaxi* newItem = itemMaxi->copy(this);
                    createItem(newItem);
                    newItem->setIsSelect(false);

                    itemsToDeleteFromSource.append(itemMaxi);
                }
            }

            // Desmarcar los movidos dentro del mismo player
            for (AudioItem* item : movedItems) {
                item->setIsSelect(false);
            }

            // Borrar los originales del otro player (CUT/PASTE)
            for (AudioItemMaxi* oldItem : itemsToDeleteFromSource) {
                if (!oldItem)
                    continue;

                if (oldItem->parentWidget() && oldItem->parentWidget()->layout()) {
                    oldItem->parentWidget()->layout()->removeWidget(oldItem);
                }

                oldItem->deleteLater();
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

        // Si no hay widget, ocultar acciones específicas
        if (!widget) {
            contentsMenu->setColorVisible(false);
            contentsMenu->setSelectAllVisible(false);
            contentsMenu->setCopyVisible(false);
            contentsMenu->setCutVisible(false);
            contentsMenu->setDeleteVisible(false);
            contentsMenu->setPropertiesVisible(false);
            contentsMenu->setPasteVisible(!this->clipboard.lista.isEmpty());

          //  contentsMenu->setLoadVisible(false);
            contentsMenu->setSaveVisible(false);
            contentsMenu->setSaveAsVisible(false);

            contentsMenu->exec(mapToGlobal(mousePos));
            return;
        }

        // Mostrar acciones por defecto
        contentsMenu->setColorVisible(true);
        contentsMenu->setSelectAllVisible(true);
        contentsMenu->setCopyVisible(true);
        contentsMenu->setCutVisible(true);
        contentsMenu->setDeleteVisible(true);
        contentsMenu->setPropertiesVisible(true);

      //  contentsMenu->setLoadVisible(true);
        contentsMenu->setSaveVisible(true);
        contentsMenu->setSaveAsVisible(true);



        // Mostrar u ocultar Paste según clipboard
        contentsMenu->setPasteVisible(!this->clipboard.lista.isEmpty());

        // Mostrar menú nuevo
        contentsMenu->exec(mapToGlobal(mousePos));

}

//************************ menus ************
void ContentsPlayer::selectAllItems()
{

    QList<AudioItemMaxi*> items = this->findChildren<AudioItemMaxi*>();

    for (AudioItemMaxi* item : std::as_const(items)) {
        item->setIsSelect(true);
    }
}



void ContentsPlayer::deleteSelected(){

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
}


// ContentsPlayer.cpp
void ContentsPlayer::copySelected()
{
    clipboard.lista.clear();

    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    bool hasSelection = false;

    for (AudioItemMaxi* item : std::as_const(items)) {
        if (item->isSelect()) {
            item->setProperty("iscut", false);
            clipboard.lista.append(item);
            hasSelection = true;
        }
    }

    // Si no hay selección, copiar item bajo el cursor
    if (!hasSelection) {
        QWidget *widget = childAt(mousePos);
        while (widget && !qobject_cast<AudioItemMaxi*>(widget))
            widget = widget->parentWidget();

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->setProperty("iscut", false);
            clipboard.lista.append(item);
        }
    }
}

// ContentsPlayer.cpp
void ContentsPlayer::cutSelected()
{
    clipboard.lista.clear();

    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    bool hasSelection = false;

    for (AudioItemMaxi* item : std::as_const(items)) {
        if (item->isSelect()) {
            item->setProperty("iscut", true);  // marcar como cortado
            clipboard.lista.append(item);
            hasSelection = true;
        }
    }

    // Si no hay selección, cortar item bajo el cursor
    if (!hasSelection) {
        QWidget *widget = childAt(mousePos);
        while (widget && !qobject_cast<AudioItemMaxi*>(widget))
            widget = widget->parentWidget();

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->setProperty("iscut", true);
            clipboard.lista.append(item);
        }
    }
}

// ContentsPlayer.cpp
void ContentsPlayer::pasteClipboard()
{
    if (clipboard.lista.empty())
        return;

    bool isCutOperation = clipboard.lista.constFirst()->property("iscut").toBool();

    for (auto it = clipboard.lista.begin(); it != clipboard.lista.end(); ) {
        if (auto *itemBase = qobject_cast<AudioItemMaxi*>(*it)) {
            AudioItemMaxi *newItem = itemBase->copy(this);
            createItem(newItem);

            newItem->setIsSelect(false); // deseleccionar pegado

            if (isCutOperation) {
                itemBase->deleteLater();
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
}

// ContentsPlayer.cpp
void ContentsPlayer::applyColor(const QColor &color)
{
    QList<AudioItemMaxi*> items = findChildren<AudioItemMaxi*>();
    bool hasSelection = false;

    // Aplicar color a los items seleccionados
    for (AudioItemMaxi* item : items) {
        if (item->isSelect()) {
            item->framecolor->setColor(color);
            hasSelection = true;
        }
    }

    // Si no hay selección, aplicar al item bajo el cursor
    if (!hasSelection) {
        QWidget *widget = childAt(mousePos);
        while (widget && !qobject_cast<AudioItemMaxi*>(widget))
            widget = widget->parentWidget();

        if (auto *item = qobject_cast<AudioItemMaxi*>(widget)) {
            item->framecolor->setColor(color);
        }
    }

    // Deseleccionar todos los items después de aplicar color
    for (AudioItemMaxi* item : items) {
        item->setIsSelect(false);
    }
}


/*void ContentsPlayer::setPlayer(Player *player)
{
    m_player = player;
}*/


