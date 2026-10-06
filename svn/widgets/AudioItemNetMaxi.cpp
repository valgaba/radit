/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/AudioItemNetMaxi.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
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
    QDialog dialog(this);
    dialog.setObjectName("OnlineRadioDialog");
    dialog.setWindowTitle(tr("Online radio"));
    dialog.setWindowIcon(QIcon(":/icons/radit.ico"));
    dialog.setMinimumWidth(450);
    auto *form = new QFormLayout(&dialog);
    form->setContentsMargins(14, 14, 14, 14);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(10);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto *name = new QLineEdit(nameFile(), &dialog);
    auto *address = new QLineEdit(url(), &dialog);
    address->setPlaceholderText("https://server.example/stream");
    address->setToolTip(tr("Direct audio stream URL, not the station website"));
    form->addRow(tr("Name"), name);
    form->addRow(tr("Stream URL"), address);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    const auto validate = [name, address, buttons]() {
        buttons->button(QDialogButtonBox::Ok)->setEnabled(!name->text().trimmed().isEmpty() &&
            MediaManager::isNetworkUrl(address->text()));
    };
    connect(name, &QLineEdit::textChanged, &dialog, validate);
    connect(address, &QLineEdit::textChanged, &dialog, validate);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    validate();
    if (dialog.exec() != QDialog::Accepted) return false;
    if (isPlaying()) {
        QWidget *owner = parentWidget();
        while (owner && !qobject_cast<Player*>(owner)) owner = owner->parentWidget();
        if (auto *player = qobject_cast<Player*>(owner)) player->stopMain();
    }
    setUrl(address->text());
    setNameFile(name->text().trimmed());
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
