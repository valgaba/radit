/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemNetPlanner.h"
#include "widgets/NetPropertiesDialog.h"
#include "widgets/Player.h"

#include <QUrl>

AudioItemNetPlanner::AudioItemNetPlanner(QWidget *parent)
    : AudioItemNetMaxi(parent)
{
    setObjectName("AudioItemNetPlanner");
    setCompactPresentation(true);
    setTiempoFile(m_connectionDurationSeconds);
}

AudioItemNetPlanner::AudioItemNetPlanner(const AudioItemNetMaxi &source, QWidget *parent)
    : AudioItemNetPlanner(parent)
{
    setUrl(source.url());
    setNameFile(source.nameFile());
    setIsSelect(source.isSelect());
    setIsPlayNext(source.isPlayNext());
    setIsPurge(source.isPurge());
    setColor(source.color());
    setToolTip(source.toolTip());
    if (source.playbackLimitSeconds() > 0.0)
        setConnectionDurationSeconds(qRound(source.playbackLimitSeconds()));
}

AudioItemMaxi *AudioItemNetPlanner::copy(QWidget *newParent) const
{
    return new AudioItemNetPlanner(*this, newParent);
}

void AudioItemNetPlanner::setConnectionDurationSeconds(int seconds)
{
    const int boundedSeconds=qBound(1,seconds,23*60*60+59*60+59);
    if (boundedSeconds==m_connectionDurationSeconds)
        return;
    m_connectionDurationSeconds=boundedSeconds;
    setTiempoFile(m_connectionDurationSeconds);
    emit connectionDurationChanged();
}

bool AudioItemNetPlanner::editStation()
{
    NetPropertiesDialog dialog(nameFile(),url(),m_connectionDurationSeconds,this);
    if (dialog.exec()!=QDialog::Accepted)
        return false;

    if (isPlaying()) {
        QWidget *owner=parentWidget();
        while (owner && !qobject_cast<Player*>(owner))
            owner=owner->parentWidget();
        if (auto *player=qobject_cast<Player*>(owner))
            player->stopMain();
    }
    setUrl(dialog.streamUrl());
    setNameFile(dialog.stationName());
    setConnectionDurationSeconds(dialog.connectionDurationSeconds());
    return true;
}
