#include "widgets/AudioItemListMaxi.h"

AudioItemListMaxi::AudioItemListMaxi(QWidget *parent)
    : AudioItemFolderMaxi(parent)
{
    setObjectName("AudioItemListMaxi");
}

AudioItemListMaxi::AudioItemListMaxi(const AudioItemListMaxi &source, QWidget *parent)
    : AudioItemListMaxi(parent)
{
    setMixSettings(source.mixEnabled(), source.mixSeconds());
    setAnnouncementTexts(source.introAnnouncement(), source.outroAnnouncement(),
                         source.announcementDelaySeconds());
    setAnnouncementsEnabled(source.announcementsEnabled());
    setAnnouncementVolumePercent(source.announcementVolumePercent());
    setAnnouncementMusicVolumePercent(source.announcementMusicVolumePercent());
    setAnnouncementRecoverySeconds(source.announcementRecoverySeconds());
    setSweeper(source.sweeperEnabled(), source.sweeperAtEntry(), source.sweeperAtExit());
    setIsPurge(source.isPurge());
    setIsPlayNext(source.isPlayNext());
    setIsLoop(source.isLoop());
    setIsSelect(source.isSelect());
    setEnabledForPlayback(source.isEnabledForPlayback());
    setColor(source.color());
    copyFolderStateFrom(source);
}

AudioItemMaxi *AudioItemListMaxi::copy(QWidget *newParent) const
{
    return new AudioItemListMaxi(*this, newParent);
}
