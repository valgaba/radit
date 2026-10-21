/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/PlannerContents.h"
#include "widgets/Planner.h"
#include "widgets/ScheduleSlot.h"
#include "widgets/AudioItem.h"
#include "widgets/AudioItemFilePlanner.h"
#include "widgets/AudioItemFileMaxi.h"
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/AudioItemFolderPlanner.h"
#include "widgets/AudioItemListMaxi.h"
#include "widgets/AudioItemListPlanner.h"
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
#include <utility>

namespace {
AudioItemMaxi *makePlannerItem(AudioItemMaxi *source, QWidget *parent)
{
    AudioItemMaxi *item=nullptr;
    if (auto *listItem=qobject_cast<AudioItemListMaxi*>(source))
        item=new AudioItemListPlanner(*listItem,parent);
    else if (auto *fileItem=qobject_cast<AudioItemFileMaxi*>(source)) {
        if (auto *plannerItem=qobject_cast<AudioItemFilePlanner*>(fileItem))
            item=plannerItem->copy(parent);
        else {
            auto *file=new AudioItemFilePlanner(parent);
            file->setNameFile(fileItem->nameFile());
            file->setFilePath(fileItem->filePath());
            file->setSecond(fileItem->second());
            file->setTiempoFile(fileItem->second());
            file->setSecondStart(fileItem->secondStart());
            item=file;
        }
    }
    else if (auto *folderItem=qobject_cast<AudioItemFolderMaxi*>(source))
        item=new AudioItemFolderPlanner(*folderItem,parent);
    else if (auto *netItem=qobject_cast<AudioItemNetMaxi*>(source))
        item=new AudioItemNetPlanner(*netItem,parent);
    else if (auto *meteoItem=qobject_cast<AudioItemMeteoClockMaxi*>(source))
        item=new AudioItemMeteoClockPlanner(*meteoItem,parent);

    if (item) {
        // Keep shared playback settings when converting Player/Standby items
        // into the compact Planner representation.
        item->setMixSettings(source->mixEnabled(),source->mixSeconds());
        if (source->playbackLimitSeconds()>0.0)
            item->setPlaybackLimitSeconds(source->playbackLimitSeconds());
        item->setIsLoop(source->isLoop());
        item->setIsPurge(source->isPurge());
        item->setIsPlayNext(source->isPlayNext());
        item->setEnabledForPlayback(source->isEnabledForPlayback());
        item->setColor(source->color());
        item->setToolTip(source->toolTip());
        item->setIsSelect(false);
    }
    return item;
}

bool isPlannerItem(AudioItemMaxi *item)
{
    return qobject_cast<AudioItemListPlanner*>(item)
        || qobject_cast<AudioItemFilePlanner*>(item)
        || qobject_cast<AudioItemFolderPlanner*>(item)
        || qobject_cast<AudioItemNetPlanner*>(item)
        || qobject_cast<AudioItemMeteoClockPlanner*>(item);
}

}

PlannerContents::PlannerContents(QWidget *parent) : ContentsPlayer(parent)
{
    connect(this,&ContentsPlayer::audioDurationsRefreshed,this,[this](int updated,int) {
        if (updated<=0)
            return;
        emit contentDurationsChanged();
        emit contentModified();
    });
}

AudioItemMaxi *PlannerContents::createFolderItem()
{
    return new AudioItemFolderPlanner(this);
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
    if (created) {
        QWidget *owner=parentWidget();
        while (owner && !qobject_cast<ScheduleSlot*>(owner))
            owner=owner->parentWidget();
        created->setPlannerEnableControlVisible(owner!=nullptr);
        if (!owner)
            created->setEnabledForPlayback(true);
        if (qobject_cast<AudioItemListMaxi*>(created))
            created->setPlannerTypeIcon(QStringLiteral(":/icons/list2.svg"));
        else if (qobject_cast<AudioItemFolderMaxi*>(created))
            created->setPlannerTypeIcon(QStringLiteral(":/icons/folder.svg"));
        else if (qobject_cast<AudioItemFileMaxi*>(created))
            created->setPlannerTypeIcon(QStringLiteral(":/icons/audiofile.svg"));
        else if (qobject_cast<AudioItemNetMaxi*>(created))
            created->setPlannerTypeIcon(QStringLiteral(":/icons/net.svg"));
        else if (qobject_cast<AudioItemMeteoClockMaxi*>(created))
            created->setPlannerTypeIcon(QStringLiteral(":/icons/meteo.svg"));
        connect(created,&AudioItemMaxi::propertiesOpened,this,[this]() {
            emit contentModified();
        });
        connect(created,&AudioItemMaxi::playbackEnabledChanged,this,[this]() {
            emit contentModified();
            emit contentDurationsChanged();
        });
    }
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
    menu.setFixedWidth(240);
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
    QAction *loadStandbyAction=nullptr;
    QAction *saveStandbyAction=nullptr;
    QAction *saveStandbyAsAction=nullptr;
    if (m_standbyFileActionsEnabled) {
        menu.addSeparator();
        loadStandbyAction=menu.addAction(tr("Load Standby"));
        saveStandbyAction=menu.addAction(tr("Save Standby"));
        saveStandbyAsAction=menu.addAction(tr("Save Standby As"));
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
    pasteAction->setEnabled(clipboard.hasItems());

    QAction *chosen=menu.exec(mapToGlobal(event->pos()));
    if (chosen==addOnlineRadioAction) {
        auto *radio=new AudioItemNetPlanner(this);
        if (radio->editStation())
            createItem(radio);
        else
            radio->deleteLater();
    }
    else if (chosen==selectAllAction) {
        selectAllItems();
        emit contentModified();
    }
    else if (chosen==unselectAllAction) {
        unSelectAllItems();
        emit contentModified();
    }
    else if (chosen==selectAction) {
        selectItems();
        emit contentModified();
    }
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
    else if (chosen && (chosen==loadStandbyAction || chosen==saveStandbyAction
                        || chosen==saveStandbyAsAction)) {
        QWidget *owner=parentWidget();
        while (owner && !qobject_cast<Planner*>(owner))
            owner=owner->parentWidget();
        if (auto *planner=qobject_cast<Planner*>(owner)) {
            if (chosen==loadStandbyAction)
                planner->loadStandbyFile();
            else
                planner->saveStandbyFile(chosen==saveStandbyAsAction);
        }
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
        if (sourceLists.size()==1 && sourceLists.constFirst()==this) {
            // Reorder within the same ScheduleSlot; normalize the insertion
            // index after removing the dragged widgets. In particular, dropping
            // at the end starts with count(), which is too large after removal.
            QList<AudioItemMaxi*> movingItems;
            for (AudioItem *source : std::as_const(draggedItems)) {
                if (auto *item=qobject_cast<AudioItemMaxi*>(source))
                    movingItems.append(item);
            }

            QList<AudioItemMaxi*> orderedItems;
            for (int i=0; i<layout->count(); ++i) {
                auto *item=qobject_cast<AudioItemMaxi*>(layout->itemAt(i)->widget());
                if (item && movingItems.contains(item))
                    orderedItems.append(item);
            }

            QWidget *targetWidget=childAt(event->position().toPoint());
            while (targetWidget && !qobject_cast<AudioItemMaxi*>(targetWidget))
                targetWidget=targetWidget->parentWidget();
            int targetIndex=targetWidget ? layout->indexOf(targetWidget) : layout->count();
            if (targetIndex<0)
                targetIndex=layout->count();

            int movingBeforeTarget=0;
            for (int i=0; i<targetIndex; ++i) {
                auto *item=qobject_cast<AudioItemMaxi*>(layout->itemAt(i)->widget());
                if (item && movingItems.contains(item))
                    ++movingBeforeTarget;
            }
            int insertIndex=qBound(0,targetIndex-movingBeforeTarget,layout->count());
            for (AudioItemMaxi *item : std::as_const(orderedItems))
                layout->removeWidget(item);
            insertIndex=qBound(0,insertIndex,layout->count());
            for (AudioItemMaxi *item : std::as_const(orderedItems)) {
                layout->insertWidget(insertIndex++,item);
                item->setParent(this);
                item->setIsSelect(false);
            }

            emit contentsModified();
            emit contentModified();
            emit contentDurationsChanged();
            event->acceptProposedAction();
            return;
        }

        // A cross-slot drop must keep Planner item subclasses. The generic
        // ContentsPlayer handler converts these to Maxi items, which leaves
        // non-Planner widgets inside a ScheduleSlot and breaks its playback/UI
        // assumptions. Clone through the Planner copy path, then remove sources.
        QWidget *targetWidget=childAt(event->position().toPoint());
        while (targetWidget && !qobject_cast<AudioItemMaxi*>(targetWidget))
            targetWidget=targetWidget->parentWidget();
        int insertIndex=targetWidget ? layout->indexOf(targetWidget) : layout->count();
        if (insertIndex<0)
            insertIndex=layout->count();

        for (AudioItem *source : std::as_const(draggedItems)) {
            auto *audioSource=qobject_cast<AudioItemMaxi*>(source);
            if (!audioSource)
                continue;

            QWidget *owner=audioSource->parentWidget();
            while (owner && !qobject_cast<ContentsPlayer*>(owner))
                owner=owner->parentWidget();
            auto *sourceContents=qobject_cast<PlannerContents*>(owner);
            if (!sourceContents)
                continue;

            AudioItemMaxi *plannerCopy=makePlannerItem(audioSource,this);
            AudioItemMaxi *created=plannerCopy ? createItem(plannerCopy) : nullptr;
            if (!created)
                continue;

            layout->removeWidget(created);
            layout->insertWidget(qBound(0,insertIndex,layout->count()),created);
            ++insertIndex;
            sourceContents->deleteItem(audioSource);
        }

        event->acceptProposedAction();
        return;
    }

    QWidget *targetWidget=childAt(event->position().toPoint());
    while (targetWidget && !qobject_cast<AudioItemMaxi*>(targetWidget))
        targetWidget=targetWidget->parentWidget();
    int insertIndex=targetWidget ? layout->indexOf(targetWidget) : layout->count();
    if (insertIndex<0)
        insertIndex=layout->count();

    for (AudioItem *source : std::as_const(draggedItems)) {
        auto *audioSource=qobject_cast<AudioItemMaxi*>(source);
        if (!audioSource)
            continue;
        AudioItemMaxi *compact=makePlannerItem(audioSource,this);
        AudioItemMaxi *created=compact ? createItem(compact) : nullptr;
        if (!created)
            continue;

        layout->removeWidget(created);
        layout->insertWidget(qBound(0,insertIndex,layout->count()),created);
        ++insertIndex;

        QWidget *owner=audioSource->parentWidget();
        while (owner && !qobject_cast<ContentsPlayer*>(owner))
            owner=owner->parentWidget();
        auto *sourceContents=qobject_cast<ContentsPlayer*>(owner);
        if (sourceContents && sourceContents!=this)
            sourceContents->deleteItem(audioSource);
    }
    event->acceptProposedAction();
}
