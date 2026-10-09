/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/ScheduleSlotOptionsDialog.h"

#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QTimeEdit>
#include <QVBoxLayout>

ScheduleSlotOptionsDialog::ScheduleSlotOptionsDialog(const QTime &initialTime, QWidget *parent,
                                                     bool disabled, bool editing,
                                                     const QString &name)
    : QDialog(parent)
{
    setObjectName("ScheduleSlotOptionsDialog");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    setModal(true);
    setWindowTitle(tr("Schedule slot options"));
    resize(360, 195);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *titleBar = new Frame(this);
    titleBar->setObjectName("framebarra");
    titleBar->setFixedHeight(25);
    auto *titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);
    auto *title = new Label(titleBar);
    title->setObjectName("PanelTitle");
    title->setText(windowTitle());
    titleLayout->addWidget(title);
    titleLayout->addStretch();
    auto *closeButton = new Button(titleBar);
    closeButton->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    closeButton->setFixedSize(15, 15);
    closeButton->SetIcon("Close-hover.svg");
    closeButton->setIconSize(QSize(15, 15));
    closeButton->setToolTip(tr("Close schedule slot options"));
    titleLayout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    root->addWidget(titleBar);

    auto *content = new QWidget(this);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(12, 10, 12, 10);
    contentLayout->setSpacing(10);
    auto *form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    m_name=new QLineEdit(content);
    m_name->setObjectName("ScheduleSlotName");
    m_name->setText(name.isEmpty()
                        ? tr("Pauta de las %1").arg(initialTime.toString("HH:mm"))
                        : name);
    form->addRow(tr("Name"),m_name);
    m_entryTime = new QTimeEdit(initialTime, content);
    m_entryTime->setObjectName("ScheduleSlotEntryTime");
    m_entryTime->setDisplayFormat("HH:mm:ss");
    m_entryTime->setTimeRange(QTime(0, 0, 0), QTime(23, 59, 59));
    form->addRow(tr("Entry time"), m_entryTime);
    m_disabled = new QCheckBox(tr("Disable this schedule slot"), content);
    m_disabled->setObjectName("ScheduleSlotDisabledCheckBox");
    m_disabled->setChecked(disabled);
    form->addRow(QString(), m_disabled);
    contentLayout->addLayout(form);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    buttons->addStretch();
    auto *cancel = new Button(content);
    cancel->setText(tr("Cancel"));
    cancel->setFixedSize(75, 24);
    auto *add = new Button(content);
    add->setObjectName("AddScheduleSlot");
    add->setText(editing ? tr("Apply") : tr("Add"));
    add->setFixedSize(75, 24);
    buttons->addWidget(cancel);
    buttons->addWidget(add);
    contentLayout->addLayout(buttons);
    root->addWidget(content, 1);

    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(add, &QPushButton::clicked, this, &QDialog::accept);
}

QTime ScheduleSlotOptionsDialog::entryTime() const
{
    return m_entryTime->time();
}

QString ScheduleSlotOptionsDialog::name() const
{
    return m_name->text().trimmed();
}

bool ScheduleSlotOptionsDialog::isDisabled() const
{
    return m_disabled->isChecked();
}
