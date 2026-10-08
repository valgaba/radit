/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemFolderPlanner.h"

AudioItemFolderPlanner::AudioItemFolderPlanner(QWidget *parent)
    : AudioItemFolderMaxi(parent)
{
    setObjectName("AudioItemFolderPlanner");
    setCompactPresentation(true);
}

AudioItemFolderPlanner::AudioItemFolderPlanner(const AudioItemFolderMaxi &source, QWidget *parent)
    : AudioItemFolderPlanner(parent)
{
    setMixSettings(source.mixEnabled(), source.mixSeconds());
    setIsPurge(source.isPurge());
    setIsPlayNext(source.isPlayNext());
    setIsLoop(source.isLoop());
    setIsSelect(source.isSelect());
    setColor(source.color());
    setToolTip(source.toolTip());
    if (!source.folderPath().isEmpty())
        setFolderPath(source.folderPath());
}

AudioItemMaxi *AudioItemFolderPlanner::copy(QWidget *newParent) const
{
    return new AudioItemFolderPlanner(*this, newParent);
}
