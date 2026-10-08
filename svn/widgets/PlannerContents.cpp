/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/PlannerContents.h"
#include "widgets/AudioItemFilePlanner.h"
#include "widgets/AudioItemFileMaxi.h"

#include <QDataStream>
#include <QDropEvent>
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
    return ContentsPlayer::createItem(item);
}

void PlannerContents::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat("application/x-audioitems")) {
        ContentsPlayer::dropEvent(event);
        return;
    }

    QByteArray data=event->mimeData()->data("application/x-audioitems");
    QDataStream stream(&data,QIODevice::ReadOnly);
    while (!stream.atEnd()) {
        quintptr address=0;
        stream >> address;
        auto *source=reinterpret_cast<AudioItem*>(address);
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
