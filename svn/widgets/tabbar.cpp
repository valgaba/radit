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
#include <QIcon>
#include <QFont>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QPainter>
#include <QStyleOptionTab>


#include "widgets/tabbar.h"
#include "widgets/TabPlayer.h"

TabBar::TabBar(QWidget *parent):QTabBar(parent){
    this->setObjectName("TabBar"); //para qss
    setAcceptDrops(true);

    QFont font;
         font.setPointSize(10);
         font.setBold(false);

         this->setFont(font);

         this->setTabsClosable(true);

         this->setCursor(QCursor(Qt::PointingHandCursor));  //cambiamos el cursor


 }



TabBar::~TabBar(){}

void TabBar::setTabColor(int index, const QColor &color)
{
    if (index < 0 || index >= count())
        return;
    setTabData(index, color.isValid() && color.alpha() > 0 ? QVariant(color) : QVariant());
    update();
}

QColor TabBar::tabColor(int index) const
{
    return tabData(index).value<QColor>();
}

void TabBar::paintEvent(QPaintEvent *event)
{
    QTabBar::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    for (int index = 0; index < count(); ++index) {
        const QColor color = tabColor(index);
        if (!color.isValid() || color.alpha() == 0 || !isTabVisible(index))
            continue;
        QStyleOptionTab option;
        initStyleOption(&option, index);
        const QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, &option, this);
        painter.save();
        painter.translate(textRect.left(), textRect.top() - 7);
        painter.scale(qMin(80, textRect.width()) / 80.0, 0.6);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawPolygon(QPolygonF{QPointF(0, 0), QPointF(50, 0), QPointF(40, 10), QPointF(0, 10)});
        painter.drawPolygon(QPolygonF{QPointF(55, 0), QPointF(65, 0), QPointF(55, 10), QPointF(45, 10)});
        painter.drawPolygon(QPolygonF{QPointF(70, 0), QPointF(80, 0), QPointF(70, 10), QPointF(60, 10)});
        painter.restore();
    }
}


void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    // Aceptamos el drag
    event->acceptProposedAction();
}

void TabBar::dragMoveEvent(QDragMoveEvent *event)
{
    // Posición del cursor dentro del TabBar
    QPoint pos = event->position().toPoint();

    // Averiguar sobre qué pestaña está el cursor
    int index = tabAt(pos);

    if (index >= 0) {
        // Cambiar automáticamente a esa pestaña
        setCurrentIndex(index);

        // Hacer que la pestaña activa sea visible

    }

    event->acceptProposedAction();
}

void TabBar::dropEvent(QDropEvent *event)
{
    auto *tabs = qobject_cast<TabPlayer*>(parentWidget());
    if (tabs && tabs->loadDroppedPlayer(event->mimeData())) event->acceptProposedAction();
    else event->ignore();
}







