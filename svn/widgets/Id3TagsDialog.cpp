/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/Id3TagsDialog.h"

#include "core/MediaManager.h"
#include "widgets/button.h"
#include "widgets/frame.h"

#include <QFileInfo>
#include <QDir>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>
#include <QVBoxLayout>
#include <QWindow>

Id3TagsDialog::Id3TagsDialog(const QString &filePath, QWidget *parent)
    : QDialog(parent), m_filePath(filePath)
{
    setObjectName("Id3TagsDialog");
    setWindowTitle(tr("ID3 tags"));
    setWindowIcon(QIcon(":/icons/settings.svg"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    resize(510, 330);

    const AudioFileMetadata metadata = MediaManager::readAudioFileMetadata(filePath);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_titleBar = new Frame(this);
    m_titleBar->setObjectName("framebarra");
    m_titleBar->setFixedHeight(25);
    m_titleBar->installEventFilter(this);
    auto *barLayout = new QHBoxLayout(m_titleBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->setSpacing(0);
    auto *title = new QLabel(m_titleBar);
    title->setObjectName("PanelTitle");
    title->setText(windowTitle());
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    barLayout->addWidget(title);
    barLayout->addStretch();
    auto *close = new Button(m_titleBar);
    close->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    close->setFixedSize(15, 15);
    close->SetIcon("Close-hover.svg");
    close->setIconSize(QSize(15, 15));
    close->setToolTip(tr("Close ID3 tags"));
    barLayout->addWidget(close);
    connect(close, &Button::clicked, this, &QDialog::reject);
    root->addWidget(m_titleBar);

    auto *content = new QWidget(this);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);
    auto *form = new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    const auto addField = [this, form](const QString &label, const QString &value,
                                       QLineEdit **field) {
        *field = new QLineEdit(value, this);
        (*field)->setClearButtonEnabled(true);
        form->addRow(label, *field);
    };
    addField(tr("Title"), metadata.title, &m_title);
    addField(tr("Artist"), metadata.artist, &m_artist);
    addField(tr("Album"), metadata.album, &m_album);
    addField(tr("Year"), metadata.year, &m_year);
    addField(tr("Genre"), metadata.genre, &m_genre);
    addField(tr("Comment"), metadata.comment, &m_comment);
    layout->addLayout(form);

    auto *path = new QLabel(QDir::toNativeSeparators(QFileInfo(filePath).absoluteFilePath()), content);
    path->setObjectName("Id3FilePath");
    path->setWordWrap(true);
    path->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(path);

    m_status = new QLabel(content);
    m_status->setObjectName("Id3Status");
    m_status->setWordWrap(true);
    layout->addWidget(m_status);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    buttons->addStretch();
    auto *cancel = new Button(content);
    cancel->setText(tr("Cancel"));
    cancel->setFixedSize(70, 24);
    auto *save = new Button(content);
    save->setText(tr("Save"));
    save->setFixedSize(70, 24);
    buttons->addWidget(cancel);
    buttons->addWidget(save);
    layout->addLayout(buttons);
    root->addWidget(content);

    connect(cancel, &Button::clicked, this, &QDialog::reject);
    connect(save, &Button::clicked, this, &Id3TagsDialog::saveTags);
    auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escape, &QShortcut::activated, this, &QDialog::reject);
}

void Id3TagsDialog::saveTags()
{
    AudioFileMetadata metadata;
    metadata.title = m_title->text();
    metadata.artist = m_artist->text();
    metadata.album = m_album->text();
    metadata.year = m_year->text();
    metadata.genre = m_genre->text();
    metadata.comment = m_comment->text();
    QString error;
    if (!MediaManager::writeAudioFileMetadata(m_filePath, metadata, &error)) {
        m_status->setText(error);
        m_status->setProperty("error", true);
        m_status->style()->unpolish(m_status);
        m_status->style()->polish(m_status);
        return;
    }
    accept();
}

bool Id3TagsDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                if (windowHandle() && windowHandle()->startSystemMove()) return true;
                m_dragging = true;
                m_dragOffset = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_dragging) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->buttons() & Qt::LeftButton) move(mouse->globalPosition().toPoint() - m_dragOffset);
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
        }
    }
    return QDialog::eventFilter(watched, event);
}
