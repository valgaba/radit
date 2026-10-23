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
    setSongsPerTurn(source.songsPerTurn());
    setAvoidArtistRepeats(source.avoidsArtistRepeats());
    setArtistRepeatDistance(source.artistRepeatDistance());
    setAnnouncementTexts(source.introAnnouncement(), source.outroAnnouncement(),
                         source.announcementDelaySeconds());
    setAnnouncementsEnabled(source.announcementsEnabled());
    setAnnouncementVolumePercent(source.announcementVolumePercent());
    setAnnouncementMusicVolumePercent(source.announcementMusicVolumePercent());
    setAnnouncementRecoverySeconds(source.announcementRecoverySeconds());
    setSweeper(source.sweeperEnabled(), source.sweeperAtEntry(), source.sweeperAtExit());
    setSweeperEveryNSongs(source.sweeperEveryNSongs());
    setIsPurge(source.isPurge());
    setIsPlayNext(source.isPlayNext());
    setIsLoop(source.isLoop());
    setIsSelect(source.isSelect());
    setEnabledForPlayback(source.isEnabledForPlayback());
    setColor(source.color());
    copyFolderStateFrom(source);
}

AudioItemMaxi *AudioItemFolderPlanner::copy(QWidget *newParent) const
{
    return new AudioItemFolderPlanner(*this, newParent);
}
