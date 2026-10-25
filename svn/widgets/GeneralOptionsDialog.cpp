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
#include <QCheckBox>
#include <QAbstractSpinBox>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFrame>
#include <QSize>
#include <QSpinBox>
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
    resize(550, 360);

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

    auto *sweeperFrame = new QFrame(content);
    sweeperFrame->setObjectName(QStringLiteral("GeneralSweeperFrame"));
    sweeperFrame->setStyleSheet(QStringLiteral(
        "QFrame#GeneralSweeperFrame { background:#181e2c; border:1px solid #4e4d7a; }"));
    auto *sweeperLayout = new QVBoxLayout(sweeperFrame);
    sweeperLayout->setContentsMargins(10, 8, 10, 9);
    sweeperLayout->setSpacing(7);
    auto *sweeperTitle = new QLabel(tr("Sweepers"), sweeperFrame);
    sweeperTitle->setStyleSheet(QStringLiteral(
        "QLabel { color:#80A4AE; background:transparent; border:none; font-weight:bold; }"));
    sweeperLayout->addWidget(sweeperTitle);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    auto *folderRow = new QWidget(sweeperFrame);
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
    sweeperLayout->addLayout(form);
    layout->addWidget(sweeperFrame);

    auto *transitionFrame = new QFrame(content);
    transitionFrame->setObjectName(QStringLiteral("GeneralScheduleTransitionFrame"));
    transitionFrame->setStyleSheet(QStringLiteral(
        "QFrame#GeneralScheduleTransitionFrame { background:#181e2c; "
        "border:1px solid #4e4d7a; }") );
    auto *transitionLayout = new QVBoxLayout(transitionFrame);
    transitionLayout->setContentsMargins(10, 8, 10, 9);
    transitionLayout->setSpacing(7);
    auto *transitionTitle = new QLabel(tr("Planner transitions"), transitionFrame);
    transitionTitle->setStyleSheet(QStringLiteral(
        "QLabel { color:#80A4AE; background:transparent; border:none; font-weight:bold; }") );
    transitionLayout->addWidget(transitionTitle);

    auto *transitionForm = new QFormLayout;
    transitionForm->setHorizontalSpacing(12);
    transitionForm->setVerticalSpacing(8);
    m_scheduleTransitionsEnabled = new QCheckBox(tr("Use transition clips before scheduled events"), transitionFrame);
    m_scheduleTransitionsEnabled->setChecked(AudioSettings::scheduleTransitionsEnabled());
    m_scheduleTransitionsEnabled->setToolTip(
        tr("Use a separate station ID or musical clip between Standby tracks, never over a song."));
    m_scheduleTransitionsEnabled->setStyleSheet(QStringLiteral(
        "QCheckBox { color:#8a8d96; spacing:6px; }"
        "QCheckBox::indicator { width:13px; height:13px; border:1px solid #5b6070; "
        "background:#202635; }"
        "QCheckBox::indicator:checked { background:#4e4d7a; border:1px solid #8a8d96; }"));
    transitionForm->addRow(m_scheduleTransitionsEnabled);

    auto *transitionFolderRow = new QWidget(transitionFrame);
    auto *transitionFolderLayout = new QHBoxLayout(transitionFolderRow);
    transitionFolderLayout->setContentsMargins(0, 0, 0, 0);
    transitionFolderLayout->setSpacing(12);
    m_scheduleTransitionFolder = new QLineEdit(AudioSettings::scheduleTransitionFolder(), transitionFolderRow);
    m_scheduleTransitionFolder->setObjectName(QStringLiteral("GeneralScheduleTransitionFolder"));
    m_scheduleTransitionFolder->setCursorPosition(0);
    auto *transitionBrowse = new Button(transitionFolderRow);
    transitionBrowse->SetIcon(QStringLiteral("folder.svg"));
    transitionBrowse->setText(QString());
    transitionBrowse->setFixedSize(28, 24);
    transitionBrowse->setIconSize(QSize(18, 18));
    transitionBrowse->setToolTip(tr("Browse..."));
    transitionBrowse->setAccessibleName(transitionBrowse->toolTip());
    transitionFolderLayout->addWidget(m_scheduleTransitionFolder, 1);
    transitionFolderLayout->addWidget(transitionBrowse);
    transitionForm->addRow(tr("Transition clips folder"), transitionFolderRow);

    const QString numericControlStyle = QStringLiteral(
        "QSpinBox { color:#80A4AE; background:#282f40; border:1px solid #3c4558; padding:3px; }"
        "QSpinBox QLineEdit { color:#80A4AE; background:transparent; border:none; padding:0px; }"
        "QSpinBox:disabled { color:#5b6070; background:#202635; }");
    m_minimumStandbyTrackSeconds = new QSpinBox(transitionFrame);
    m_minimumStandbyTrackSeconds->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_minimumStandbyTrackSeconds->setStyleSheet(numericControlStyle);
    m_minimumStandbyTrackSeconds->setRange(1, 600);
    m_minimumStandbyTrackSeconds->setSuffix(tr(" s"));
    m_minimumStandbyTrackSeconds->setValue(AudioSettings::minimumStandbyTrackSeconds());
    m_minimumStandbyTrackSeconds->setToolTip(
        tr("If the next song would have less time than this before the scheduled event, try a transition clip."));
    transitionForm->addRow(tr("Minimum music before a schedule"), m_minimumStandbyTrackSeconds);
    m_scheduleTransitionLateToleranceSeconds = new QSpinBox(transitionFrame);
    m_scheduleTransitionLateToleranceSeconds->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_scheduleTransitionLateToleranceSeconds->setStyleSheet(numericControlStyle);
    m_scheduleTransitionLateToleranceSeconds->setRange(0, 60);
    m_scheduleTransitionLateToleranceSeconds->setSuffix(tr(" s"));
    m_scheduleTransitionLateToleranceSeconds->setValue(AudioSettings::scheduleTransitionLateToleranceSeconds());
    m_scheduleTransitionLateToleranceSeconds->setToolTip(
        tr("A scheduled event may wait this long for a transition clip to finish."));
    transitionForm->addRow(tr("Maximum schedule delay"), m_scheduleTransitionLateToleranceSeconds);
    transitionLayout->addLayout(transitionForm);
    layout->addWidget(transitionFrame);

    const auto updateTransitionControls = [this]() {
        const bool enabled = m_scheduleTransitionsEnabled->isChecked();
        m_scheduleTransitionFolder->setEnabled(enabled);
        m_minimumStandbyTrackSeconds->setEnabled(enabled);
        m_scheduleTransitionLateToleranceSeconds->setEnabled(enabled);
    };
    connect(m_scheduleTransitionsEnabled, &QCheckBox::toggled,
            this, [updateTransitionControls](bool) { updateTransitionControls(); });
    updateTransitionControls();
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
    connect(transitionBrowse, &Button::clicked, this, [this]() {
        const QString path = QFileDialog::getExistingDirectory(
            this, tr("Select transition clips folder"), m_scheduleTransitionFolder->text());
        if (!path.isEmpty())
            m_scheduleTransitionFolder->setText(QDir::toNativeSeparators(path));
    });
    connect(cancel, &Button::clicked, this, &QWidget::close);
    connect(save, &Button::clicked, this, [this]() {
        const QString path = m_sweeperFolder->text().trimmed();
        if (!path.isEmpty() && !QFileInfo(path).isDir()) {
            QMessageBox::warning(this, tr("General options"),
                                 tr("The common sweeper folder does not exist."));
            return;
        }
        const QString transitionPath = m_scheduleTransitionFolder->text().trimmed();
        if (m_scheduleTransitionsEnabled->isChecked()
            && !transitionPath.isEmpty() && !QFileInfo(transitionPath).isDir()) {
            QMessageBox::warning(this, tr("General options"),
                                 tr("The transition clips folder does not exist."));
            return;
        }
        AudioSettings::setSweeperFolder(path);
        AudioSettings::setScheduleTransitionFolder(transitionPath);
        AudioSettings::setScheduleTransitionsEnabled(m_scheduleTransitionsEnabled->isChecked());
        AudioSettings::setMinimumStandbyTrackSeconds(m_minimumStandbyTrackSeconds->value());
        AudioSettings::setScheduleTransitionLateToleranceSeconds(
            m_scheduleTransitionLateToleranceSeconds->value());
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
