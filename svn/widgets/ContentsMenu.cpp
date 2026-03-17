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

#include "ContentsMenu.h"
#include "ContentsPlayer.h"
#include "widgets/AudioItemFileMaxi.h"
#include <QWidgetAction>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPainter>

struct ColorButton { QString name; QColor color; };

ContentsMenu::ContentsMenu(ContentsPlayer *parent)
    : QMenu(parent), player(parent)
{
    setFixedWidth(200);
    setupAddMenu();
    setupColorMenu();
    setupActions();
    connectActions();
}

void ContentsMenu::showAt(const QPoint &pos) {
    exec(player->mapToGlobal(pos));
}

void ContentsMenu::setColorVisible(bool visible) {
    if (colorAction) colorAction->setVisible(visible);
}

void ContentsMenu::setSelectAllVisible(bool visible) {
    if (selectallAction) selectallAction->setVisible(visible);
}

void ContentsMenu::setCopyVisible(bool visible) {
    if (copyAction) copyAction->setVisible(visible);
}

void ContentsMenu::setCutVisible(bool visible) {
    if (cutAction) cutAction->setVisible(visible);
}

void ContentsMenu::setDeleteVisible(bool visible) {
    if (deleteAction) deleteAction->setVisible(visible);
}

void ContentsMenu::setPropertiesVisible(bool visible) {
    if (propertiesAction) propertiesAction->setVisible(visible);
}

void ContentsMenu::setPasteVisible(bool visible) {
    if (pasteAction) pasteAction->setVisible(visible);
}




// ---------------------------
// Submenu "Añadir"
void ContentsMenu::setupAddMenu() {
    addMenuAction = createAddAction();
}

// Submenu "Color"
void ContentsMenu::setupColorMenu() {
    colorAction = createColorAction();
}

// Acciones generales
void ContentsMenu::setupActions() {
       selectallAction = new QAction("SelectAll", this);
       cutAction = new QAction("Cut", this);
       copyAction = new QAction("Copy", this);
       pasteAction = new QAction("Paste", this);
       deleteAction = new QAction("Delete", this);
       propertiesAction = new QAction("Properties", this);

       QMenu::addAction(addMenuAction);
       addSeparator();
       QMenu::addAction(selectallAction);
       QMenu::addAction(cutAction);
       QMenu::addAction(copyAction);
       QMenu::addAction(pasteAction);
       QMenu::addAction(deleteAction);
       addSeparator();
       QMenu::addAction(propertiesAction);
       addSeparator();
       QMenu::addAction(colorAction);
}

// Conectar acciones
void ContentsMenu::connectActions() {
    connect(selectallAction, &QAction::triggered, player, &ContentsPlayer::selectAllItems);
    connect(deleteAction, &QAction::triggered, player, &ContentsPlayer::deleteSelected);
    connect(copyAction, &QAction::triggered, player, &ContentsPlayer::copySelected);
    connect(cutAction, &QAction::triggered, player, &ContentsPlayer::cutSelected);
    connect(pasteAction, &QAction::triggered, player, &ContentsPlayer::pasteClipboard);
   // connect(propertiesAction, &QAction::triggered, player, &ContentsPlayer::showPropertiesForHoveredItem);
}

// Crear submenú "Añadir"
QAction* ContentsMenu::createAddAction() {
    QAction *action = new QAction("Añadir", this);
    QMenu *submenu = new QMenu(this);

    QAction *addAudiofile = new QAction("Audio File", submenu);
    QAction *addAudioFolder = new QAction("Audio Folder", submenu);
    QAction *addNeturl = new QAction("Net Url", submenu);
    QAction *addGroup = new QAction("Group", submenu);

    addAudiofile->setIcon(QIcon(":/icons/audiofile.svg"));
    addAudioFolder->setIcon(QIcon(":/icons/folder.svg"));
    addNeturl->setIcon(QIcon(":/icons/net.svg"));

    submenu->addAction(addAudiofile);
    submenu->addAction(addAudioFolder);
    submenu->addAction(addNeturl);
    submenu->addAction(addGroup);

    action->setMenu(submenu);

    connect(addAudiofile, &QAction::triggered, player, [this]{
        player->createItem(new AudioItemFileMaxi(player));
    });

    return action;
}

// Crear submenú "Color"
QAction* ContentsMenu::createColorAction() {
    QAction *action = new QAction("Color Item", this);
    QMenu *submenu = new QMenu(this);

    QWidgetAction *widgetAction = new QWidgetAction(submenu);
    QWidget *widget = new QWidget(submenu);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    QVector<ColorButton> colors = {
        {"Red", QColor(231, 76, 60)},
        {"Orange", QColor(230, 126, 34)},
        {"Green", QColor(124, 179, 66)},
        {"Blue", QColor(0, 172, 193)},
        {"Cyan", QColor(52, 152, 219)},
        {"Pink", QColor(216, 27, 96)}
    };

    for (const auto &c : colors) {
        QPushButton *btn = new QPushButton(createColorIcon(c.color), "");
        btn->setFlat(true);
        btn->setToolTip(c.name);
        btn->setFixedSize(32, 32);
        btn->setIconSize(QSize(32,32));
        layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, player, [this, c]{
               player->applyColor(c.color);
               this->close();
           });
    }

    widgetAction->setDefaultWidget(widget);
    submenu->addAction(widgetAction);
    action->setMenu(submenu);

    return action;
}

// Icono redondo de color
QIcon ContentsMenu::createColorIcon(const QColor &color) {
    QPixmap pixmap(24,24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(4, 4, 16, 16, 4, 4);
    return QIcon(pixmap);
}




