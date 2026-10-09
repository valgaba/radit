/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemFilePlanner.h"

AudioItemFilePlanner::AudioItemFilePlanner(QWidget *parent)
    : AudioItemFileMaxi(parent)
{
    setObjectName("AudioItemFilePlanner");
    setCompactPresentation(true);
}

AudioItemMaxi *AudioItemFilePlanner::copy(QWidget *newParent) const
{
    auto *item=new AudioItemFilePlanner(newParent);
    item->setNameFile(nameFile());
    item->setFilePath(filePath());
    item->setSecond(second());
    item->setTiempoFile(second());
    item->setSecondStart(secondStart());
    item->setToolTip(toolTip());
    return item;
}
