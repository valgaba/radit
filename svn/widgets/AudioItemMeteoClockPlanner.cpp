/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemMeteoClockPlanner.h"

AudioItemMeteoClockPlanner::AudioItemMeteoClockPlanner(QWidget *parent)
    : AudioItemMeteoClockMaxi(parent)
{
    setObjectName("AudioItemMeteoClockPlanner");
    setCompactPresentation(true);
}

AudioItemMeteoClockPlanner::AudioItemMeteoClockPlanner(const AudioItemMeteoClockMaxi &source,
                                                       QWidget *parent)
    : AudioItemMeteoClockPlanner(parent)
{
    if (!source.filePath().isEmpty())
        setVoicePackPath(source.filePath());
    setNameFile(source.nameFile());
    setIsLoop(source.isLoop());
    setIsPurge(source.isPurge());
    setIsPlayNext(source.isPlayNext());
    setIsSelect(source.isSelect());
    setColor(source.color());
    setToolTip(source.toolTip());
    setAnnouncementOptions(source.announcesTime(),source.announcesWeather());
}

AudioItemMaxi *AudioItemMeteoClockPlanner::copy(QWidget *newParent) const
{
    return new AudioItemMeteoClockPlanner(*this, newParent);
}
