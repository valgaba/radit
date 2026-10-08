/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/ScheduleSlot.h"
#include "widgets/PlannerContents.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"
#include "widgets/scrollbar.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>

ScheduleSlot::ScheduleSlot(QWidget *parent) : Frame(parent)
{
    setObjectName("ScheduleSlot");
    setFixedHeight(100);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    auto *layout=new QVBoxLayout(this);
    layout->setContentsMargins(1,1,1,1);
    layout->setSpacing(0);

    auto *header=new Frame(this);
    header->setObjectName("framebarra");
    header->setFixedHeight(25);
    auto *headerLayout=new QHBoxLayout(header);
    headerLayout->setContentsMargins(4,0,0,0);
    headerLayout->setSpacing(4);

    m_time=new Label(header);
    m_time->setObjectName("ScheduleSlotTime");
    m_time->setText("10:00");
    headerLayout->addWidget(m_time);
    headerLayout->addStretch(1);

    auto *closeButton=new Button(header);
    closeButton->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    closeButton->setFixedSize(15,15);
    closeButton->SetIcon("Close-hover.svg");
    closeButton->setIconSize(QSize(15,15));
    closeButton->setToolTip(tr("Close schedule slot"));
    closeButton->setAccessibleName(closeButton->toolTip());
    headerLayout->addWidget(closeButton);

    m_scrollArea=new QScrollArea(this);
    m_scrollArea->setObjectName("ScheduleSlotContainer");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBar(new ScrollBar);
    m_scrollArea->setHorizontalScrollBar(new ScrollBar);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_contents=new PlannerContents(m_scrollArea);
    m_contents->setObjectName("ScheduleSlotContents");
    m_scrollArea->setWidget(m_contents);

    layout->addWidget(header);
    layout->addWidget(m_scrollArea,1);
}

void ScheduleSlot::setTimeText(const QString &time)
{
    m_time->setText(time);
}
