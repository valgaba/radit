/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/PlannerContents.h"
#include "widgets/AudioItem.h"
#include "widgets/AudioItemFilePlanner.h"
#include "widgets/AudioItemFileMaxi.h"
#include "widgets/menu.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDataStream>
#include <QDropEvent>
#include <QIcon>
#include <QMimeData>

PlannerContents::PlannerContents(QWidget *parent) : ContentsPlayer(parent)
{
}

AudioItemMaxi *PlannerContents::createItem(AudioItemMaxi *item)
{
    auto *fileItem=qobject_cast<AudioItemFileMaxi*>(item);
    if (fileItem && !qobject_cast<AudioItemFilePlanner*>(item)) {
        auto *compact=new AudioItemFilePlanner(this);
        compact->setNameFile(fileItem->nameFile());
        compact->setFilePath(fileItem->filePath());
        compact->setSecond(fileItem->second());
        compact->setTiempoFile(fileItem->second());
        compact->setSecondStart(fileItem->secondStart());
        compact->setToolTip(fileItem->toolTip());
        delete fileItem;
        item=compact;
    }
    AudioItemMaxi *created=ContentsPlayer::createItem(item);
    emit contentDurationsChanged();
    if (created)
        emit contentAdded();
    return created;
}

void PlannerContents::deleteItem(AudioItemMaxi *item)
{
    if (!item)
        return;
    ContentsPlayer::deleteItem(item);
    emit contentDurationsChanged();
}

void PlannerContents::contextMenuEvent(QContextMenuEvent *event)
{
    setContextMenuPosition(event->pos());

    QWidget *target=childAt(event->pos());
    while (target && !qobject_cast<AudioItemMaxi*>(target))
        target=target->parentWidget();
    bool hasSelection=false;
    for (AudioItemMaxi *item : findChildren<AudioItemMaxi*>())
        hasSelection|=item->isSelect();

    Menu menu(this);
    menu.setFixedWidth(190);
    QAction *selectAllAction=menu.addAction(QIcon(":/icons/Selectall.svg"),tr("Select All"));
    QAction *unselectAllAction=menu.addAction(QIcon(":/icons/unselect.svg"),tr("Unselect All"));
    QAction *selectAction=menu.addAction(tr("Select"));
    menu.addSeparator();
    QAction *copyAction=menu.addAction(QIcon(":/icons/ActionCopy.svg"),tr("Copy"));
    QAction *cutAction=menu.addAction(QIcon(":/icons/ActionCut.svg"),tr("Cut"));
    QAction *pasteAction=menu.addAction(QIcon(":/icons/ActionPaste.svg"),tr("Paste"));
    menu.addSeparator();
    QAction *deleteAction=menu.addAction(QIcon(":/icons/Remove.svg"),tr("Delete"));
    menu.addSeparator();
    QAction *propertiesAction=menu.addAction(QIcon(":/icons/properties.svg"),tr("Properties"));

    const bool hasTarget=target!=nullptr;
    selectAllAction->setEnabled(!findChildren<AudioItemMaxi*>().isEmpty());
    unselectAllAction->setEnabled(hasSelection);
    selectAction->setEnabled(hasTarget);
    copyAction->setEnabled(hasSelection||hasTarget);
    cutAction->setEnabled(hasSelection||hasTarget);
    deleteAction->setEnabled(hasSelection||hasTarget);
    pasteAction->setEnabled(!clipboard.lista.isEmpty());

    QAction *chosen=menu.exec(mapToGlobal(event->pos()));
    if (chosen==selectAllAction)
        selectAllItems();
    else if (chosen==unselectAllAction)
        unSelectAllItems();
    else if (chosen==selectAction)
        selectItems();
    else if (chosen==copyAction)
        copySelected();
    else if (chosen==cutAction)
        cutSelected();
    else if (chosen==pasteAction)
        pasteClipboard();
    else if (chosen==deleteAction)
        deleteSelected();
    else if (chosen==propertiesAction) {
        // Reserved for future ScheduleSlot content properties.
    }
    event->accept();
}

void PlannerContents::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("application/x-audioitems")) {
        ContentsPlayer::dropEvent(event);
        return;
    }

    QByteArray data=event->mimeData()->data("application/x-audioitems");
    QDataStream stream(&data,QIODevice::ReadOnly);
    QList<AudioItem*> draggedItems;
    QList<PlannerContents*> sourceLists;
    bool allFromPlannerLists=true;
    while (!stream.atEnd()) {
        quintptr address=0;
        stream >> address;
        auto *source=reinterpret_cast<AudioItem*>(address);
        if (!source) {
            allFromPlannerLists=false;
            continue;
        }
        draggedItems.append(source);
        QWidget *owner=source->parentWidget();
        while (owner && !qobject_cast<ContentsPlayer*>(owner))
            owner=owner->parentWidget();
        auto *plannerOwner=qobject_cast<PlannerContents*>(owner);
        if (!plannerOwner) {
            allFromPlannerLists=false;
        } else if (!sourceLists.contains(plannerOwner)) {
            sourceLists.append(plannerOwner);
        }
    }

    if (!draggedItems.isEmpty() && allFromPlannerLists) {
        // Reorder within a slot or move between Planner slots without duplicating.
        ContentsPlayer::dropEvent(event);
        for (PlannerContents *sourceList : sourceLists) {
            if (sourceList!=this)
                emit sourceList->contentDurationsChanged();
        }
        emit contentDurationsChanged();
        return;
    }

    for (AudioItem *source : draggedItems) {
        auto *fileItem=qobject_cast<AudioItemFileMaxi*>(source);
        if (!fileItem)
            continue;

        AudioItemFilePlanner *compact=nullptr;
        if (auto *plannerItem=qobject_cast<AudioItemFilePlanner*>(source)) {
            compact=qobject_cast<AudioItemFilePlanner*>(plannerItem->copy(this));
        } else {
            compact=new AudioItemFilePlanner(this);
            compact->setNameFile(fileItem->nameFile());
            compact->setFilePath(fileItem->filePath());
            compact->setSecond(fileItem->second());
            compact->setTiempoFile(fileItem->second());
            compact->setSecondStart(fileItem->secondStart());
            compact->setToolTip(fileItem->toolTip());
        }
        createItem(compact);
    }
    event->acceptProposedAction();
}
