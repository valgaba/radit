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

    // Aceptar operaciones de drag and drop
   // setAcceptDrops(true);
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
    if (!(event->buttons() & Qt::LeftButton))
            return;

        if ((event->pos() - dragStartPosition).manhattanLength()
            < QApplication::startDragDistance())
            return;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        QWidget *container = parentWidget();

        QList<AudioItem*> selected;
        QList<AudioItem*> items = container->findChildren<AudioItem*>();

        for (auto *item : std::as_const(items)) {
            if (item->isSelect())
                selected.append(item);
        }

        // si no hay seleccionados arrastramos solo este
        if (selected.isEmpty()) {
            stream << reinterpret_cast<quintptr>(this);
        }
        else {
            for (auto *item : std::as_const(selected))
                stream << reinterpret_cast<quintptr>(item);
        }

        mimeData->setData("application/x-audioitems", data);
        drag->setMimeData(mimeData);

        QPixmap pixmap(size());
        render(&pixmap);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos());

        drag->exec(Qt::MoveAction);
}

//***********************************************

void AudioItem::setFilePath(const QString &filePath)
{
    m_filePath = filePath;
}

void AudioItem::setSecond(double second)
{
    m_second = second;
}

QString AudioItem::filePath() const
{
    return m_filePath;
}

double AudioItem::second() const
{
    return m_second;
}



void AudioItem::setIsSelect(bool value)
{
    m_isSelect = value;
}

bool AudioItem::isSelect() const
{
    return m_isSelect;
}


