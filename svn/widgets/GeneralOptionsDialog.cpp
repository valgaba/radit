/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/GeneralOptionsDialog.h"

#include "core/AudioSettings.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"

#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSize>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWidget>

GeneralOptionsDialog::GeneralOptionsDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("GeneralOptionsDialog"));
    setWindowTitle(tr("General options"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/radit.ico")));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    resize(510, 220);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_titleBar = new Frame(this);
    m_titleBar->setObjectName(QStringLiteral("framebarra"));
    m_titleBar->setFixedHeight(25);
    m_titleBar->installEventFilter(this);
    auto *barLayout = new QHBoxLayout(m_titleBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->setSpacing(0);
    auto *title = new Label(m_titleBar);
    title->setObjectName(QStringLiteral("PanelTitle"));
    title->setText(windowTitle());
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    barLayout->addWidget(title);
    barLayout->addStretch();
    auto *close = new Button(m_titleBar);
    close->setObjectName(QStringLiteral("CloseGeneralOptions"));
    close->setStyleSheet(QStringLiteral("QPushButton { border: none; background: transparent; padding: 0px; }"));
    close->setFixedSize(15, 15);
    close->SetIcon(QStringLiteral("Close-hover.svg"));
    close->setIconSize(QSize(15, 15));
    connect(close, &Button::clicked, this, &QWidget::close);
    barLayout->addWidget(close);
    root->addWidget(m_titleBar);

    auto *content = new QWidget(this);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(10);
    root->addWidget(content, 1);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    auto *folderRow = new QWidget(content);
    auto *folderLayout = new QHBoxLayout(folderRow);
    folderLayout->setContentsMargins(0, 0, 0, 0);
    folderLayout->setSpacing(12);
    m_sweeperFolder = new QLineEdit(AudioSettings::sweeperFolder(), folderRow);
    m_sweeperFolder->setObjectName(QStringLiteral("GeneralSweeperFolder"));
    m_sweeperFolder->setCursorPosition(0);
    auto *browse = new Button(folderRow);
    browse->SetIcon(QStringLiteral("folder.svg"));
    browse->setText(QString());
    browse->setFixedSize(28, 24);
    browse->setIconSize(QSize(18, 18));
    browse->setToolTip(tr("Browse..."));
    browse->setAccessibleName(browse->toolTip());
    folderLayout->addWidget(m_sweeperFolder, 1);
    folderLayout->addWidget(browse);
    form->addRow(tr("Sweeper folder"), folderRow);
    layout->addLayout(form);
    layout->addStretch();

    auto *buttons = new QHBoxLayout;
    buttons->addStretch();
    auto *cancel = new Button(this);
    cancel->setText(tr("Cancel"));
    cancel->setFixedSize(70, 24);
    auto *save = new Button(this);
    save->setText(tr("Save"));
    save->setObjectName(QStringLiteral("SaveGeneralOptions"));
    save->setFixedSize(70, 24);
    buttons->addWidget(cancel);
    buttons->addWidget(save);
    layout->addLayout(buttons);

    connect(browse, &Button::clicked, this, [this]() {
        const QString path = QFileDialog::getExistingDirectory(
            this, tr("Select common sweeper folder"), m_sweeperFolder->text());
        if (!path.isEmpty())
            m_sweeperFolder->setText(QDir::toNativeSeparators(path));
    });
    connect(cancel, &Button::clicked, this, &QWidget::close);
    connect(save, &Button::clicked, this, [this]() {
        const QString path = m_sweeperFolder->text().trimmed();
        if (!path.isEmpty() && !QFileInfo(path).isDir()) {
            QMessageBox::warning(this, tr("General options"),
                                 tr("The common sweeper folder does not exist."));
            return;
        }
        AudioSettings::setSweeperFolder(path);
        accept();
    });
    auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escape, &QShortcut::activated, this, &QWidget::close);
}

bool GeneralOptionsDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleBar && event->type() == QEvent::MouseButtonPress) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton) {
            m_dragOffset = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
            return true;
        }
    } else if (watched == m_titleBar && event->type() == QEvent::MouseMove) {
        auto *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->buttons() & Qt::LeftButton) {
            move(mouse->globalPosition().toPoint() - m_dragOffset);
            return true;
        }
    }
    return QDialog::eventFilter(watched, event);
}
