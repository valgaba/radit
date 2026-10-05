/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Radit. If not, see <http://www.gnu.org/licenses/>.
*/

#include "widgets/TapPlayerMenu.h"

#include <QAction>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPainter>
#include <QPushButton>
#include <QWidgetAction>

TapPlayerMenu::TapPlayerMenu(QWidget *parent) : Menu(parent)
{
    setFixedWidth(220);
    const auto action = [this](const QString &text, const QString &icon,
                               void (TapPlayerMenu::*request)()) {
        auto *result = addAction(QIcon(":/icons/" + icon), text);
        connect(result, &QAction::triggered, this, request);
        return result;
    };

    auto *add = action(tr("Add list"), "Add.svg", &TapPlayerMenu::addListRequested);
    add->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C));
    action(tr("Delete list"), "Remove.svg", &TapPlayerMenu::deleteListRequested);
    action(tr("Rename list"), "properties.svg", &TapPlayerMenu::renameListRequested);
    addSeparator();
    action(tr("Load player"), "Load.svg", &TapPlayerMenu::loadPlayerRequested);
    action(tr("Save player"), "Save.svg", &TapPlayerMenu::savePlayerRequested);
    action(tr("Save player as"), "SaveAs.svg", &TapPlayerMenu::savePlayerAsRequested);
    addSeparator();
    setupColorPalette();
}

void TapPlayerMenu::setupColorPalette()
{
    auto *paletteAction = new QWidgetAction(this);
    auto *palette = new QWidget(this);
    auto *layout = new QHBoxLayout(palette);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    const auto button = [this, palette, layout](const QString &name,
                                               const QColor &color, const QIcon &icon) {
        auto *result = new QPushButton(palette);
        result->setFlat(true);
        result->setFixedSize(22, 22);
        result->setIconSize(QSize(16, 16));
        result->setIcon(icon);
        result->setToolTip(name);
        result->setAccessibleName(name);
        result->setCursor(Qt::PointingHandCursor);
        layout->addWidget(result);
        connect(result, &QPushButton::clicked, this, [this, color]() {
            close();
            emit colorRequested(color);
        });
    };
    button(tr("No Color"), Qt::transparent, QIcon(":/icons/CheckBox.svg"));
    const struct { const char *name; QColor color; } colors[] = {
        {"Red", QColor(231, 76, 60)},
        {"Orange", QColor(230, 126, 34)},
        {"Green", QColor(124, 179, 66)},
        {"Blue", QColor(0, 172, 193)},
        {"Cyan", QColor(52, 152, 219)},
        {"Pink", QColor(216, 27, 96)}
    };
    for (const auto &color : colors)
        button(tr(color.name), color.color, colorIcon(color.color));
    paletteAction->setDefaultWidget(palette);
    addAction(paletteAction);
}

QIcon TapPlayerMenu::colorIcon(const QColor &color)
{
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(4, 4, 16, 16, 4, 4);
    return QIcon(pixmap);
}
