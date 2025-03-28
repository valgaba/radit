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

#include <QDebug>


#include <QScreen>
#include <QDragEnterEvent>

#include "widgets/FrameView.h"


FrameView::FrameView(QWidget *parent):Frame(parent){

    this->setObjectName("FrameView"); //para qss

    externalwindow = new ExternalWindow;




    layout = new QVBoxLayout;
    this->setLayout(layout);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);

    frame=new QFrame;
    //frame->setMinimumSize(QSize(400, 300));
    //frame->setMaximumSize(QSize(400, 300));
    //frame->setStyleSheet("QFrame { background-color: black; border: 0px; }");



    QVBoxLayout *framelayout = new QVBoxLayout(frame);
    framelayout->setContentsMargins(0, 0, 0, 0);


    label=new QLabel;

    label->setStyleSheet("QLabel { background-color: black; border: none;  }"); // Establecemos el fondo transparente

    QFont font;
    font.setPointSize(50);

    // Establecer la fuente para el QLabel
     label->setFont(font);
     label->setText("00:00:00.0")  ;



      framelayout->addWidget(label);
      layout->addWidget(frame);





    menu = new Menu(this);
    menu->setFixedWidth(200); // Establecer anchura del menú en píxeles

    QAction *screenAction = new QAction("Proyectar en pantalla", this);

    // Crear el submenú con la lista de pantallas
    Menu *submenu = new Menu(this);

    QAction *action = new QAction(submenu);
    action->setText(QString("Proyectar en ventana")); // Nombre de la acción
    submenu->addAction(action);

    connect(action, &QAction::triggered, this, [=]() {

        externalwindow->show();
    });





    QList<QScreen*> screens = QGuiApplication::screens();

    for (int i = 0; i < screens.size(); ++i) {
        QAction *action = new QAction(submenu);
        action->setText(QString("Pantalla %1").arg(i + 1)); // Nombre de la acción
        action->setData(i); // Almacenamos el índice de la pantalla en la propiedad 'data' de la acción
        submenu->addAction(action);

        connect(action, &QAction::triggered, this, [=]() {
            // Obtener el índice de la pantalla desde la acción seleccionada
            int screenIndex = action->data().toInt();
            QScreen *screen = screens.at(screenIndex);
            qDebug() <<screen->geometry();


        });




    }
    // Agregar el submenú a la opción "Añadir"
    screenAction ->setMenu(submenu);



    //screenAction ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C)); // Establecer atajo de teclado



    // connect(copyAction, &QAction::triggered, this, &MyWidget::copyActionTriggered);


    menu->addAction(screenAction );
    menu->addSeparator(); // Añadir un separador
   // menu->addAction(copyAction);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FrameView::updateCountdown);

     timer->start(10); // Temporizador que se ejecuta cada segundo

 //externalwindow->show();

}




FrameView::~FrameView(){
    timer->stop();

    delete externalwindow;
}



void FrameView::contextMenuEvent(QContextMenuEvent *event)
{

    menu->popup(mapToGlobal(event->pos()));
    menu->exec(mapToGlobal(event->pos()));

}


void FrameView::updateCountdown() {
    // Reducir el tiempo restante
    timeLeft--;

    // Actualizar el texto del QLabel
    label->setText("00:00." + QString::number(timeLeft));
    //QPixmap pixmap = this->grab();
    QPixmap pixmap = this->frame->grab();


    // Renderiza el contenido del QFrame en la ventana externa
    externalwindow->renderFrame(pixmap);

}



