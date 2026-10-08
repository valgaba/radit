/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemMeteoClockMaxi.h"
#include "widgets/MeteoClock.h"
#include "widgets/Player.h"
#include "core/LocutionPack.h"
#include <QFileInfo>
#include <QFileDialog>
#include <cmath>

AudioItemMeteoClockMaxi::AudioItemMeteoClockMaxi(QWidget *parent) : AudioItemMaxi(parent)
{
    setObjectName("AudioItemMeteoClockMaxi");setNameFile(tr("MeteoClock"));setSecond(0);
    setFolderPresentation(tr("METEO"),false);
    connect(propertiesButton(),&Button::clicked,this,[this]() {
        const QString path=QFileDialog::getOpenFileName(this,tr("MeteoClock voice pack"),filePath(),tr("Voice packs (*.zip)"));
        if (path.isEmpty()) return;
        if (isPlaying()) {
            QWidget *owner=parentWidget();while(owner && !qobject_cast<Player*>(owner))owner=owner->parentWidget();
            if(auto *player=qobject_cast<Player*>(owner))player->stopMain();
        }
        setVoicePackPath(path);
    });
}
bool AudioItemMeteoClockMaxi::isLoading() const {return m_pack && m_pack->isLoading();}
void AudioItemMeteoClockMaxi::setVoicePackPath(const QString &path)
{
    if(m_pack)disconnect(m_pack.get(),nullptr,this,nullptr);
    setFilePath(QFileInfo(path).absoluteFilePath());setNameFile(tr("MeteoClock — %1").arg(QFileInfo(path).completeBaseName()));
    setSecond(0);setSecondStart(0);m_clips.clear();m_announcement.clear();m_error.clear();setToolTip(filePath());
    m_pack=LocutionPack::open(filePath());
    connect(m_pack.get(),&LocutionPack::finished,this,[this](){updatePackState();emit voicePackLoaded();});
    updatePackState();
}
void AudioItemMeteoClockMaxi::updatePackState()
{
    if (!m_pack || m_pack->isLoading()) {setFolderPresentation(tr("Loading..."),false);return;}
    m_error=m_pack->error();setToolTip(m_error.isEmpty()?filePath():filePath()+"\n"+m_error);
    setFolderPresentation(m_pack->isReady()?tr("METEO"):tr("Invalid ZIP"),m_pack->isReady());
}
QStringList AudioItemMeteoClockMaxi::clipNames(const QTime &time,double temperature,int humidity)
{
    if(!time.isValid() || !std::isfinite(temperature) || temperature < -100 || temperature > 100 || humidity<0 || humidity>100)return {};
    QStringList names{QString("HRS%1%2.mp3").arg(time.hour(),2,10,QLatin1Char('0')).arg(time.minute()==0?"_O":"")};
    if(time.minute()!=0)names.append(QString("MIN%1.mp3").arg(time.minute(),2,10,QLatin1Char('0')));
    const int degrees=qRound(temperature);
    names.append(QString("TMP%1%2.mp3").arg(degrees<0?"N":"").arg(qAbs(degrees),3,10,QLatin1Char('0')));
    names.append(QString("HUM%1.mp3").arg(humidity,3,10,QLatin1Char('0')));
    return names;
}
bool AudioItemMeteoClockMaxi::fail(const QString &message)
{
    m_error=message;m_clips.clear();setToolTip(filePath()+"\n"+message);setFolderPresentation(tr("Unavailable"),true);return false;
}
bool AudioItemMeteoClockMaxi::preparePlayback()
{
    if(!m_pack || !m_pack->isReady())return fail(m_pack && !m_pack->error().isEmpty()?m_pack->error():tr("Voice pack is not ready."));
    const auto readings=MeteoClock::currentReadings();
    if(!MeteoClock::hasFreshReadings()) {
        const bool refreshRequested=MeteoClock::requestCurrentReadings();
        return fail(refreshRequested
            ? tr("Waiting for fresh weather readings.")
            : tr("Weather readings are unavailable. Select a location in MeteoClock and wait for the weather update."));
    }
    const QTime time=QTime::currentTime();
    const auto names=clipNames(time,readings.temperature,readings.humidity);
    if(names.isEmpty())return fail(tr("Weather readings are outside the supported range."));
    QStringList clips;
    for(const auto &name:names) {
        const QString path=m_pack->clip(name);
        if(path.isEmpty())return fail(tr("The voice pack has no recording for %1.").arg(name));
        clips.append(path);
    }
    m_clips=clips;m_error.clear();setSecondStart(0);
    m_announcement=tr("%1 · %2 °C · %3 %").arg(time.toString("HH:mm")).arg(qRound(readings.temperature)).arg(readings.humidity);
    setToolTip(filePath()+"\n"+readings.location+"\n"+m_announcement);
    return true;
}
bool AudioItemMeteoClockMaxi::loadPreparedPlayback(MediaManager *manager)
{
    QString error;
    if(!manager->loadAudioSequence(m_clips,&error))return fail(error);
    setSecond(manager->getDuration());setTiempoFile(second());return true;
}
QString AudioItemMeteoClockMaxi::playbackName() const {return m_announcement.isEmpty()?nameFile():nameFile()+" — "+m_announcement;}
AudioItemMaxi *AudioItemMeteoClockMaxi::copy(QWidget *parent) const
{
    auto *item=new AudioItemMeteoClockMaxi(parent);item->setVoicePackPath(filePath());item->setNameFile(nameFile());
    item->setIsLoop(isLoop());item->setIsPurge(isPurge());item->setIsPlayNext(isPlayNext());item->setIsSelect(isSelect());item->setColor(color());
    return item;
}
