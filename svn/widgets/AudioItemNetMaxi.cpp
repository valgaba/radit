/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemNetMaxi.h"
#include "widgets/NetPropertiesDialog.h"
#include <QUrl>
#include "widgets/Player.h"

AudioItemNetMaxi::AudioItemNetMaxi(QWidget *parent) : AudioItemMaxi(parent)
{
    setObjectName("AudioItemNetMaxi");
    setSecond(0);
    setNameFile(tr("Online radio"));
    setLiveStreamPresentation();
    connect(propertiesButton(), &Button::clicked, this, [this]() { editStation(); });
}

bool AudioItemNetMaxi::setUrl(const QString &text)
{
    if (!MediaManager::isNetworkUrl(text)) return false;
    const QString address = QString::fromUtf8(QUrl(text.trimmed()).toEncoded());
    setFilePath(address);
    setToolTip(address);
    return true;
}

bool AudioItemNetMaxi::editStation()
{
    NetPropertiesDialog dialog(nameFile(),url(),-1,this);
    if (dialog.exec()!=QDialog::Accepted)
        return false;
    if (isPlaying()) {
        QWidget *owner = parentWidget();
        while (owner && !qobject_cast<Player*>(owner)) owner = owner->parentWidget();
        if (auto *player = qobject_cast<Player*>(owner)) player->stopMain();
    }
    setUrl(dialog.streamUrl());
    setNameFile(dialog.stationName());
    return true;
}

AudioItemMaxi *AudioItemNetMaxi::copy(QWidget *newParent) const
{
    auto *item = new AudioItemNetMaxi(newParent);
    item->setUrl(url());
    item->setNameFile(nameFile());
    item->setIsSelect(isSelect());
    item->setIsPlayNext(isPlayNext());
    item->setIsPurge(isPurge());
    item->setColor(color());
    return item;
}
