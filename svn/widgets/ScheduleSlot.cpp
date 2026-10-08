/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/ScheduleSlot.h"
#include "widgets/PlannerContents.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"
#include "widgets/menu.h"
#include "widgets/scrollbar.h"

#include <QAction>
#include <QHBoxLayout>
#include <QEvent>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QTime>
#include <QWidgetAction>
#include <QVBoxLayout>

namespace {
QIcon createColorIcon(const QColor &color)
{
    QPixmap pixmap(24,24);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(4,4,16,16,4,4);
    return QIcon(pixmap);
}
}

ScheduleSlot::ScheduleSlot(QWidget *parent) : Frame(parent)
{
    setObjectName("ScheduleSlot");
    setFixedHeight(100);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    setStyleSheet("QFrame#ScheduleSlot { background-color: #282020; }");

    auto *layout=new QVBoxLayout(this);
    layout->setContentsMargins(1,1,1,1);
    layout->setSpacing(0);

    auto *header=new Frame(this);
    m_header=header;
    header->setObjectName("framebarra");
    header->setFixedHeight(25);
    header->installEventFilter(this);
    header->setContextMenuPolicy(Qt::CustomContextMenu);
    auto *headerLayout=new QHBoxLayout(header);
    headerLayout->setContentsMargins(4,0,0,0);
    headerLayout->setSpacing(4);

    m_time=new Label(header);
    m_time->setObjectName("ScheduleSlotTime");
    m_time->setText("10:00");
    m_time->installEventFilter(this);
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
    connect(closeButton,&QPushButton::clicked,this,[this]() {
        emit closeRequested(this);
    });
    connect(header,&QWidget::customContextMenuRequested,this,[this,header](const QPoint &position) {
        Menu menu(header);
        menu.setFixedWidth(190);
        QAction *copyAction=menu.addAction(QIcon(":/icons/ActionCopy.svg"),tr("Copy"));
        QAction *cutAction=menu.addAction(QIcon(":/icons/ActionCut.svg"),tr("Cut"));
        menu.addSeparator();
        auto *paletteAction=new QWidgetAction(&menu);
        auto *paletteWidget=new QWidget(&menu);
        auto *paletteLayout=new QHBoxLayout(paletteWidget);
        paletteLayout->setContentsMargins(2,2,2,2);
        paletteLayout->setSpacing(4);

        auto *noColorButton=new QPushButton(paletteWidget);
        noColorButton->setIcon(QIcon(":/icons/CheckBox.svg"));
        noColorButton->setToolTip(tr("No color"));
        noColorButton->setFlat(true);
        noColorButton->setIconSize(QSize(16,16));
        paletteLayout->addWidget(noColorButton);
        connect(noColorButton,&QPushButton::clicked,this,[this,&menu]() {
            setAccentColor(QColor());
            menu.close();
        });

        const QVector<QPair<QString,QColor>> colors={
            {tr("Red"),QColor(231,76,60)},
            {tr("Orange"),QColor(230,126,34)},
            {tr("Green"),QColor(124,179,66)},
            {tr("Blue"),QColor(0,172,193)},
            {tr("Cyan"),QColor(52,152,219)},
            {tr("Pink"),QColor(216,27,96)}
        };
        for (const auto &color : colors) {
            auto *button=new QPushButton(paletteWidget);
            button->setIcon(createColorIcon(color.second));
            button->setToolTip(color.first);
            button->setFlat(true);
            button->setIconSize(QSize(16,16));
            paletteLayout->addWidget(button);
            connect(button,&QPushButton::clicked,this,[this,&menu,color]() {
                setAccentColor(color.second);
                menu.close();
            });
        }
        paletteAction->setDefaultWidget(paletteWidget);
        menu.addAction(paletteAction);
        menu.addSeparator();
        QAction *propertiesAction=menu.addAction(QIcon(":/icons/properties.svg"),tr("Properties"));
        QAction *chosen=menu.exec(header->mapToGlobal(position));
        if (chosen==copyAction) emit copyRequested(this);
        else if (chosen==cutAction) emit cutRequested(this);
        else if (chosen==propertiesAction) emit propertiesRequested(this);
    });

    m_scrollArea=new QScrollArea(this);
    m_scrollArea->setObjectName("ScheduleSlotContainer");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBar(new ScrollBar);
    m_scrollArea->setHorizontalScrollBar(new ScrollBar);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QPalette slotPalette=m_scrollArea->palette();
    slotPalette.setColor(QPalette::Window,QColor("#282020"));
    m_scrollArea->setPalette(slotPalette);
    m_scrollArea->setAutoFillBackground(true);
    m_scrollArea->viewport()->setPalette(slotPalette);
    m_scrollArea->viewport()->setAutoFillBackground(true);

    m_contents=new PlannerContents(m_scrollArea);
    m_contents->setObjectName("ScheduleSlotContents");
    m_contents->setPalette(slotPalette);
    m_contents->setAutoFillBackground(true);
    m_scrollArea->setWidget(m_contents);

    layout->addWidget(header);
    layout->addWidget(m_scrollArea,1);
}

void ScheduleSlot::setTimeText(const QString &time)
{
    const QTime parsed=QTime::fromString(time,"HH:mm");
    if (parsed.isValid())
        setEntryTime(parsed.hour()*60+parsed.minute());
    else
        m_time->setText(time);
}

void ScheduleSlot::setEntryTime(int minutesAfterMidnight)
{
    const int updatedMinute=qBound(0,minutesAfterMidnight,24*60-1);
    m_time->setText(QTime(0,0).addSecs(updatedMinute*60).toString("HH:mm"));
    if (updatedMinute==m_entryMinute)
        return;
    m_entryMinute=updatedMinute;
    emit entryTimeChanged();
}

void ScheduleSlot::setAccentColor(const QColor &color)
{
    m_accentColor=color;
    if (m_accentColor.isValid()) {
        m_header->setStyleSheet(QStringLiteral("QFrame#framebarra { background-color: %1; }")
                                    .arg(m_accentColor.name(QColor::HexRgb)));
    } else {
        m_header->setStyleSheet(QString());
    }
}

bool ScheduleSlot::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched==m_header || watched==m_time)
        && event->type()==QEvent::MouseButtonDblClick) {
        emit focusRequested(this);
    }
    return Frame::eventFilter(watched,event);
}
