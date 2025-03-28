/* This file is part of Radit.
   Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <QApplication>
#include "widgets/AudioItem.h"

AudioItem::AudioItem(QWidget *parent): QWidget(parent){

    setMinimumSize(100, 50); // Tamaño mínimo para evitar problemas visuales


    // Aceptar operaciones de drag and drop
    setAcceptDrops(true);
}

void AudioItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Guardar la posición inicial del ratón para el arrastre
        dragStartPosition = event->pos();
    }
}

void AudioItem::mouseMoveEvent(QMouseEvent *event)
{
    // Solo procesar si es el botón izquierdo
    if (!(event->buttons() & Qt::LeftButton)) return;

    // Comprobar si se ha movido lo suficiente para considerar arrastre
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

    // Crear la operación de arrastre
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText("AudioItem");
    drag->setMimeData(mimeData);

    // Crear una imagen del widget para el efecto visual de arrastre
    QPixmap pixmap(this->size());
    if (!pixmap.isNull()) {
        this->render(&pixmap);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos());
    } else {
        // Alternativa si no se puede crear el pixmap
        drag->setPixmap(QPixmap(1, 1));
    }

    // Ejecutar la operación de arrastre
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}

void AudioItem::dragEnterEvent(QDragEnterEvent *event)
{
    // Aceptar solo si el formato es texto plano
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
}

void AudioItem::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain")) {
        // Obtener el widget que se está arrastrando
        QWidget *source = qobject_cast<QWidget*>(event->source());

        if (source && source != this) {
            // Obtener el layout de destino (donde se suelta)
            QVBoxLayout *targetLayout = qobject_cast<QVBoxLayout*>(this->parentWidget()->layout());

            if (targetLayout) {
                // Remover el widget de su posición original
                if (source->parentWidget()) {
                    if (QLayout *sourceLayout = source->parentWidget()->layout()) {
                        sourceLayout->removeWidget(source);
                    }
                }

                // Insertar en la nueva posición (antes del widget actual)
                int targetIndex = targetLayout->indexOf(this);
                targetLayout->insertWidget(targetIndex, source);

                // Establecer el nuevo padre del widget
                source->setParent(this->parentWidget());

                event->acceptProposedAction();
            }
        }
    }
}


//***********************************************

//***********************************************
void AudioItem::setdayweek(int dayweek){
    this->dayweek=dayweek;
}

void AudioItem::sethour(int hour){
    this->hour=hour;
}
void AudioItem::setminute(int minute){
    this->minute=minute;

}
void AudioItem::setsecond(int second){
    this->second=second;

}

void AudioItem::setfilePath(QString filePath){
    this->filePath=filePath;

}

void AudioItem::setfilesecond(double filesecond){
    this->filesecond=filesecond;
}


int AudioItem::getdayweek()const {
    return this->dayweek;
}

int AudioItem::gethour()const{
    return this->hour;
}
int AudioItem::getminute()const{
    return this->minute;
}

int AudioItem::getsecond()const{
    return this->second;
}

QString AudioItem::getfilePath()const{
    return this->filePath;
}
double AudioItem::getfilesecond()const{
    return this->filesecond;
}







