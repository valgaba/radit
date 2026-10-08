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
#include <QStyle>
#include <QTime>
#include <QTimer>
#include <QWidgetAction>
#include <QVBoxLayout>
#include <cmath>

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

    m_toggleContents=new Button(header);
    m_toggleContents->setObjectName("ScheduleSlotToggleContents");
    m_toggleContents->setFixedSize(23,23);
    m_toggleContents->setText(QStringLiteral("▼"));
    m_toggleContents->setToolTip(tr("Hide schedule contents"));
    m_toggleContents->setAccessibleName(m_toggleContents->toolTip());
    headerLayout->addWidget(m_toggleContents);

    m_time=new Label(header);
    m_time->setObjectName("ScheduleSlotTime");
    m_time->setText("10:00");
    m_time->installEventFilter(this);
    headerLayout->addWidget(m_time);

    m_priorityIndicator=new Label(header);
    m_priorityIndicator->setObjectName("ScheduleSlotPriorityIndicator");
    m_priorityIndicator->setText(tr("Priority"));
    m_priorityIndicator->setToolTip(tr("This schedule slot has priority"));
    m_priorityIndicator->setVisible(false);
    headerLayout->addWidget(m_priorityIndicator);

    m_upcomingBlinkTimer=new QTimer(this);
    m_upcomingBlinkTimer->setInterval(500);
    connect(m_upcomingBlinkTimer,&QTimer::timeout,this,[this]() {
        m_blinkPhase=!m_blinkPhase;
        updateHeaderAppearance();
    });

    headerLayout->addStretch(1);

    m_duration=new Label(header);
    m_duration->setObjectName("ScheduleSlotDuration");
    m_duration->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    m_duration->setFixedWidth(72);
    m_duration->setText("00:00:00");
    headerLayout->addWidget(m_duration);

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
        QAction *deleteAction=menu.addAction(QIcon(":/icons/Remove.svg"),tr("Delete"));
        menu.addSeparator();
        QAction *propertiesAction=menu.addAction(QIcon(":/icons/settings.svg"),tr("Properties"));
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
            {tr("Red"),QColor("#8F3545")},
            {tr("Orange"),QColor("#9A5A2E")},
            {tr("Green"),QColor("#4A714A")},
            {tr("Blue"),QColor("#3C5C8A")},
            {tr("Cyan"),QColor("#286F78")},
            {tr("Pink"),QColor("#813E67")}
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
        QAction *chosen=menu.exec(header->mapToGlobal(position));
        if (chosen==copyAction) emit copyRequested(this);
        else if (chosen==cutAction) emit cutRequested(this);
        else if (chosen==deleteAction) emit closeRequested(this);
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

    connect(m_toggleContents,&QPushButton::clicked,this,[this]() {
        m_contentsVisible=!m_contentsVisible;
        m_scrollArea->setVisible(m_contentsVisible);
        setFixedHeight(m_contentsVisible ? m_timelineHeight : m_header->height()+2);
        m_toggleContents->setText(m_contentsVisible ? QStringLiteral("▼") : QStringLiteral("▶"));
        m_toggleContents->setToolTip(m_contentsVisible
                                         ? tr("Hide schedule contents")
                                         : tr("Show schedule contents"));
        m_toggleContents->setAccessibleName(m_toggleContents->toolTip());
    });

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

void ScheduleSlot::setTotalDuration(double seconds, bool known)
{
    m_totalDurationSeconds=seconds;
    m_totalDurationKnown=known && std::isfinite(seconds) && seconds>=0.0;
    if (!m_totalDurationKnown) {
        m_duration->setText(QStringLiteral("--:--:--"));
        return;
    }

    const qint64 totalSeconds=static_cast<qint64>(std::floor(seconds));
    const qint64 hours=totalSeconds/3600;
    const qint64 minutes=(totalSeconds%3600)/60;
    const qint64 remainingSeconds=totalSeconds%60;
    m_duration->setText(QStringLiteral("%1:%2:%3")
        .arg(hours,2,10,QLatin1Char('0'))
        .arg(minutes,2,10,QLatin1Char('0'))
        .arg(remainingSeconds,2,10,QLatin1Char('0')));
}

void ScheduleSlot::setAccentColor(const QColor &color)
{
    m_accentColor=color;
    updateHeaderAppearance();
}

void ScheduleSlot::updateHeaderAppearance()
{
    if (m_disabled) {
        m_header->setStyleSheet(QStringLiteral("QFrame#framebarra { background-color: #343846; }"));
    } else if (m_upcoming && m_blinkPhase) {
        m_header->setStyleSheet(QStringLiteral("QFrame#framebarra { background-color: #80A4AE; }"));
    } else if (m_accentColor.isValid()) {
        m_header->setStyleSheet(QStringLiteral("QFrame#framebarra { background-color: %1; }")
                                    .arg(m_accentColor.name(QColor::HexRgb)));
    } else {
        m_header->setStyleSheet(QString());
    }
}

void ScheduleSlot::setTimelineHeight(int height)
{
    m_timelineHeight=qMax(m_header->height()+2,height);
    setFixedHeight(m_contentsVisible ? m_timelineHeight : m_header->height()+2);
}

void ScheduleSlot::setScheduleDisabled(bool disabled)
{
    if (m_disabled==disabled)
        return;

    m_disabled=disabled;
    setProperty("scheduleDisabled",m_disabled);
    setAccentColor(m_accentColor);
    QPalette slotPalette=m_scrollArea->palette();
    const QColor bodyColor=m_disabled ? QColor("#20242e") : QColor("#282020");
    slotPalette.setColor(QPalette::Window,bodyColor);
    m_scrollArea->setPalette(slotPalette);
    m_scrollArea->viewport()->setPalette(slotPalette);
    m_contents->setPalette(slotPalette);
    style()->unpolish(this);
    style()->polish(this);
    for (QWidget *child : findChildren<QWidget*>()) {
        child->style()->unpolish(child);
        child->style()->polish(child);
        child->update();
    }
    update();
    emit disabledChanged();
}

void ScheduleSlot::setPriority(bool priority)
{
    if (m_priority==priority)
        return;
    m_priority=priority;
    m_priorityIndicator->setVisible(m_priority);
}

void ScheduleSlot::setUpcoming(bool upcoming)
{
    if (m_upcoming==upcoming)
        return;

    m_upcoming=upcoming;
    m_blinkPhase=upcoming;
    if (m_upcoming) {
        m_upcomingBlinkTimer->start();
    } else {
        m_upcomingBlinkTimer->stop();
    }
    updateHeaderAppearance();
}

bool ScheduleSlot::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched==m_header || watched==m_time)
        && event->type()==QEvent::MouseButtonDblClick) {
        emit focusRequested(this);
    }
    return Frame::eventFilter(watched,event);
}
