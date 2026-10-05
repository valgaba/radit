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
#include "widgets/TapPlayerMenu.h"
#include "widgets/tabbar.h"
#include "widgets/container.h"

TabPlayer::TabPlayer(QWidget *parent):Tab(parent){
   // this->setObjectName("TabPlayer"); //para qss
    //  this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor

    setContextMenuPolicy(Qt::DefaultContextMenu); // Habilitar la política de menú contextual predeterminada

    TabBar *tabbar=new TabBar(this);
    tabbar->setProperty("playerTabs", true);
    this->setTabBar(tabbar);



    menu = new TapPlayerMenu(this);

    connect(menu, &TapPlayerMenu::colorRequested, this, [this, tabbar](const QColor &color) {
        const int index = m_colorTarget ? indexOf(m_colorTarget.data()) : currentIndex();
        if (index >= 0)
            tabbar->setTabColor(index, color);
    });



    connect(menu, &TapPlayerMenu::addListRequested, this, [=]() {
        Container *container = new Container;

        int index = this->addTab(container, "noname");
        this->setTabToolTip(index, "noname"); //  el ToolTip
        this->setCurrentIndex(index);      // seleccionarla automáticamente
    });



    connect(tabbar, &TabBar::tabCloseRequested, this, [=](int index){
        closeTab(index);
    });

    connect(menu, &TapPlayerMenu::deleteListRequested, this, [=]() {
        closeTab(this->currentIndex());
    });

    connect(menu, &TapPlayerMenu::renameListRequested, this, [=]() {
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
            QString tabName = QString("List %1").arg(i);

            int index = this->addTab(container, tabName);
            this->setTabToolTip(index, tabName); //  el ToolTip
            if (i == 1) this->setCurrentIndex(index); // Seleccionar la primera por defecto


        }



}



TabPlayer::~TabPlayer(){}

void TabPlayer::closeTab(int index){

    if (index == -1) return;

    if (this->count() <= 1) return; // si solo queda una pestaña no se puede borrar




       QMessageBox::StandardButton reply;
       reply = QMessageBox::question(
           this,
           "Cerrar Pestaña",
           "¿Seguro que quieres cerrar esta pestaña?",
           QMessageBox::Yes | QMessageBox::No
       );

       if (reply != QMessageBox::Yes)
           return;

       QWidget *widgetToRemove = this->widget(index);
       if (!widgetToRemove)
           return;

       //  Buscar todos los items de audio en esta pestaña
       auto items = widgetToRemove->findChildren<AudioItemMaxi*>();

       for (AudioItemMaxi* item : std::as_const(items)) {
           if (!item) continue;

           //  Si está reproduciendo → parar
           if (item->isPlaying()) {

               // buscar player que lo está reproduciendo
               QWidget *w = item;
               Player *player = nullptr;

               while (w) {
                   player = qobject_cast<Player*>(w);
                   if (player) break;
                   w = w->parentWidget();
               }

               if (player) {
                   player->stopMain();
               }
           }
       }

       this->removeTab(index);
       widgetToRemove->deleteLater();

}




void TabPlayer::contextMenuEvent(QContextMenuEvent *event)
{
    const int hoveredIndex = tabBar()->tabAt(tabBar()->mapFromGlobal(event->globalPos()));
    m_colorTarget = widget(hoveredIndex >= 0 ? hoveredIndex : currentIndex());
    menu->exec(event->globalPos());
    m_colorTarget.clear();
    event->accept();

}


