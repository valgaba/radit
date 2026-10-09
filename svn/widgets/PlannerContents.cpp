/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/PlannerContents.h"
#include "widgets/Planner.h"
#include "widgets/AudioItem.h"
#include "widgets/AudioItemFilePlanner.h"
#include "widgets/AudioItemFileMaxi.h"
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/AudioItemFolderPlanner.h"
#include "widgets/AudioItemMeteoClockMaxi.h"
#include "widgets/AudioItemMeteoClockPlanner.h"
#include "widgets/AudioItemNetMaxi.h"
#include "widgets/AudioItemNetPlanner.h"
#include "widgets/menu.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDataStream>
#include <QDropEvent>
#include <QIcon>
#include <QMimeData>

namespace {
AudioItemMaxi *makePlannerItem(AudioItemMaxi *source, QWidget *parent)
{
    if (auto *fileItem=qobject_cast<AudioItemFileMaxi*>(source)) {
        if (auto *plannerItem=qobject_cast<AudioItemFilePlanner*>(fileItem))
            return plannerItem->copy(parent);
        auto *item=new AudioItemFilePlanner(parent);
        item->setNameFile(fileItem->nameFile());
        item->setFilePath(fileItem->filePath());
        item->setSecond(fileItem->second());
        item->setTiempoFile(fileItem->second());
        item->setSecondStart(fileItem->secondStart());
        item->setToolTip(fileItem->toolTip());
        return item;
    }
    if (auto *folderItem=qobject_cast<AudioItemFolderMaxi*>(source))
        return new AudioItemFolderPlanner(*folderItem,parent);
    if (auto *netItem=qobject_cast<AudioItemNetMaxi*>(source))
        return new AudioItemNetPlanner(*netItem,parent);
    if (auto *meteoItem=qobject_cast<AudioItemMeteoClockMaxi*>(source))
        return new AudioItemMeteoClockPlanner(*meteoItem,parent);
    return nullptr;
}

bool isPlannerItem(AudioItemMaxi *item)
{
    return qobject_cast<AudioItemFilePlanner*>(item)
        || qobject_cast<AudioItemFolderPlanner*>(item)
        || qobject_cast<AudioItemNetPlanner*>(item)
        || qobject_cast<AudioItemMeteoClockPlanner*>(item);
}
}

PlannerContents::PlannerContents(QWidget *parent) : ContentsPlayer(parent)
{
}

AudioItemMaxi *PlannerContents::createItem(AudioItemMaxi *item)
{
    if (item && !isPlannerItem(item)) {
        if (AudioItemMaxi *compact=makePlannerItem(item,this)) {
            delete item;
            item=compact;
        }
    }
    AudioItemMaxi *created=ContentsPlayer::createItem(item);
    if (auto *netItem=qobject_cast<AudioItemNetPlanner*>(created)) {
        connect(netItem,&AudioItemNetPlanner::connectionDurationChanged,this,[this]() {
            emit contentDurationsChanged();
        });
    }
    emit contentDurationsChanged();
    if (created)
        emit contentAdded();
    return created;
}

void PlannerContents::deleteItem(AudioItemMaxi *item)
{
    if (!item)
        return;
    QWidget *owner=parentWidget();
    while (owner && !qobject_cast<Planner*>(owner))
        owner=owner->parentWidget();
    if (auto *planner=qobject_cast<Planner*>(owner))
        planner->prepareForContentRemoval(this,item);
    ContentsPlayer::deleteItem(item);
    emit contentDurationsChanged();
}

void PlannerContents::contextMenuEvent(QContextMenuEvent *event)
{
    setContextMenuPosition(event->pos());

    QWidget *target=childAt(event->pos());
    while (target && !qobject_cast<AudioItemMaxi*>(target))
        target=target->parentWidget();
    auto *contextItem=qobject_cast<AudioItemMaxi*>(target);
    const auto items=findChildren<AudioItemMaxi*>();
    QList<AudioItemMaxi*> selectedItems;
    for (AudioItemMaxi *item : items) {
        if (item->isSelect())
            selectedItems.append(item);
    }
    const bool hasContent=!items.isEmpty();
    const bool hasSelection=!selectedItems.isEmpty();
    const bool hasActionTarget=contextItem!=nullptr;

    Menu menu(this);
    menu.setFixedWidth(190);
    QAction *addOnlineRadioAction=menu.addAction(QIcon(":/icons/net.svg"),tr("Add online radio"));
    menu.addSeparator();
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
    QAction *propertiesAction=menu.addAction(QIcon(":/icons/settings.svg"),tr("Properties"));
    QAction *loadListAction=nullptr;
    QAction *saveListAction=nullptr;
    if (m_listFileActionsEnabled) {
        menu.addSeparator();
        loadListAction=menu.addAction(tr("Load list"));
        saveListAction=menu.addAction(tr("Save list"));
    }

    const bool hasTarget=contextItem!=nullptr;
    selectAllAction->setEnabled(hasContent);
    unselectAllAction->setEnabled(hasSelection);
    selectAction->setVisible(hasTarget);
    selectAction->setEnabled(hasTarget);
    copyAction->setVisible(hasTarget);
    cutAction->setVisible(hasTarget);
    deleteAction->setVisible(hasActionTarget);
    propertiesAction->setVisible(hasTarget);
    copyAction->setEnabled(hasActionTarget);
    cutAction->setEnabled(hasActionTarget);
    deleteAction->setEnabled(hasActionTarget);
    propertiesAction->setEnabled(hasActionTarget);
    pasteAction->setEnabled(!clipboard.lista.isEmpty());

    QAction *chosen=menu.exec(mapToGlobal(event->pos()));
    if (chosen==addOnlineRadioAction) {
        auto *radio=new AudioItemNetPlanner(this);
        if (radio->editStation())
            createItem(radio);
        else
            radio->deleteLater();
    }
    else if (chosen==selectAllAction)
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
        AudioItemMaxi *item=contextItem;
        if (!item && !selectedItems.isEmpty())
            item=selectedItems.constFirst();
        if (item)
            item->triggerProperties();
    }
    else if (loadListAction && chosen==loadListAction)
        loadItems();
    else if (saveListAction && chosen==saveListAction)
        saveItems();
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
        auto *audioSource=qobject_cast<AudioItemMaxi*>(source);
        if (!audioSource)
            continue;
        if (AudioItemMaxi *compact=makePlannerItem(audioSource,this))
            createItem(compact);
    }
    event->acceptProposedAction();
}
