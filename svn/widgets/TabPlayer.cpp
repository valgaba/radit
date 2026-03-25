/* This file is part of Radit.
   Copyright 2023, Victor Algaba <victorengine@gmail.com> www.radit.org

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
#include <QDragEnterEvent>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>

#include "widgets/TabPlayer.h"
#include "widgets/tabbar.h"
#include "widgets/container.h"

TabPlayer::TabPlayer(QWidget *parent):Tab(parent){
   // this->setObjectName("TabPlayer"); //para qss
    //  this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor

    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada

    TabBar *tabbar=new TabBar(this);
    this->setTabBar(tabbar);

    menu = new Menu(this);
    menu->setFixedWidth(200); // Establecer anchura del menú en píxeles

    QAction *addAction = new QAction("Add list", this);
    QAction *removeAction = new QAction("Delete list", this);
    QAction *renameAction = new QAction("Rename list", this);

    addAction->setIcon(QIcon(":/icons/Add.svg"));
    removeAction->setIcon(QIcon(":/icons/Remove.svg"));

    addAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C)); // Establecer atajo de teclado


    menu->addAction(addAction);
    menu->addAction(removeAction);
    menu->addAction(renameAction);



    connect(addAction, &QAction::triggered, this, [=]() {
        Container *container = new Container;

        int index = this->addTab(container, "noname");
        this->setCurrentIndex(index);      // seleccionarla automáticamente
    });



    connect(tabbar, &TabBar::tabCloseRequested, this, [=](int index){
        closeTab(index);
    });

    connect(removeAction, &QAction::triggered, this, [=]() {
        closeTab(this->currentIndex());
    });

    connect(renameAction, &QAction::triggered, this, [=]() {
        int index = this->currentIndex();
            if (index == -1) return;

            QString currentName = this->tabText(index);

            // Crear diálogo
            QInputDialog dialog(this);
            dialog.setWindowTitle("Renombrar pestaña");
            dialog.setLabelText("");       // el QLabel todavía existe
            dialog.setTextValue(currentName);
            dialog.setFixedSize(200, 100);  // ancho x alto

            // Ocultar el QLabel para eliminar el espacio
            QList<QLabel*> labels = dialog.findChildren<QLabel*>();
            for (int i = 0; i < labels.size(); ++i) {
                labels[i]->hide();
            }

            // Seleccionar automáticamente todo el texto
            QLineEdit* lineEdit = dialog.findChild<QLineEdit*>();
            if (lineEdit) {
                lineEdit->selectAll();
            }

            // Ejecutar el diálogo
            if (dialog.exec() == QDialog::Accepted) {
                QString newName = dialog.textValue().trimmed();
                if (!newName.isEmpty()) {
                    this->setTabText(index, newName);
                }
            }
    });



    // **Crear 3 pestañas de ejemplo**
        for (int i = 1; i <= 3; ++i) {
            Container *container = new Container;
            int index = this->addTab(container, QString("Pestaña %1").arg(i));
            if (i == 1) this->setCurrentIndex(index); // Seleccionar la primera por defecto
        }



}



TabPlayer::~TabPlayer(){}

void TabPlayer::closeTab(int index){

    if (index == -1) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this,
        "Cerrar Pestaña",
        "¿Seguro que quieres cerrar esta pestaña?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {

        QWidget *widgetToRemove = this->widget(index);
          if (!widgetToRemove) return;
          this->removeTab(index);
          delete widgetToRemove;

    }

}





void TabPlayer::contextMenuEvent(QContextMenuEvent *event)
{


    menu->exec(mapToGlobal(event->pos()));

}

