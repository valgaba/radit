/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/Planner.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"
#include "widgets/ContentsPlayer.h"
#include "widgets/PlannerContents.h"
#include "widgets/ScheduleSlot.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QButtonGroup>
#include <QStackedWidget>

namespace {
class PlannerWeekTabs final : public Frame
{
public:
    explicit PlannerWeekTabs(QWidget *parent, QList<ContentsPlayer *> &dayContents)
        : Frame(parent)
    {
        auto *layout=new QVBoxLayout(this);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);

        auto *tabRow=new QHBoxLayout;
        tabRow->setContentsMargins(3,3,3,3);
        tabRow->setSpacing(3);
        auto *group=new QButtonGroup(this);
        group->setExclusive(true);
        m_pages=new QStackedWidget(this);
        m_pages->setObjectName("PlannerDayPages");

        const QStringList days={tr("Lunes"),tr("Martes"),tr("Miércoles"),tr("Jueves"),
                                tr("Viernes"),tr("Sábado"),tr("Domingo")};
        for (int index=0; index<days.size(); ++index) {
            auto *button=new Button(this);
            button->setObjectName("PlannerDayTab");
            button->setText(days.at(index));
            button->setCheckable(true);
            button->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
            button->setFixedHeight(27);
            group->addButton(button,index);
            tabRow->addWidget(button,1);

            QWidget *dayPage = nullptr;
            ScheduleSlot *slot = nullptr;
            if (index == 0) {
                dayPage=new QWidget(m_pages);
                dayPage->setObjectName("PlannerMondaySlots");
                auto *slotsLayout=new QVBoxLayout(dayPage);
                slotsLayout->setContentsMargins(0,0,0,0);
                slotsLayout->setSpacing(4);

                slot=new ScheduleSlot(dayPage);
                slotsLayout->addWidget(slot);
                auto *secondSlot=new ScheduleSlot(dayPage);
                secondSlot->setTimeText("11:00");
                slotsLayout->addWidget(secondSlot);
                slotsLayout->addStretch(1);
            } else {
                slot=new ScheduleSlot(m_pages);
                dayPage=slot;
            }
            dayContents.append(slot->contents());
            m_pages->addWidget(dayPage);
            connect(button,&QPushButton::clicked,m_pages,[this,index]() {
                m_pages->setCurrentIndex(index);
            });
            if (index == 0)
                button->setChecked(true);
        }

        layout->addLayout(tabRow);
        layout->addWidget(m_pages,1);
    }

private:
    QStackedWidget *m_pages = nullptr;
};
}

Planner::Planner(QWidget *parent) : Frame(parent)
{
    setObjectName("Planner");
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);

    auto *root=new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    auto *titleBar=new Frame(this);
    titleBar->setObjectName("framebarra");
    titleBar->setFixedHeight(25);
    auto *titleLayout=new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(0,0,0,0);
    titleLayout->setSpacing(0);

    auto *title=new Label(titleBar);
    title->setObjectName("PanelTitle");
    title->setText(tr("Planner"));
    titleLayout->addWidget(title);
    titleLayout->addStretch();

    auto *close=new Button(titleBar);
    close->setFixedSize(15,15);
    close->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    close->SetIcon("Close-hover.svg");
    close->setToolTip(tr("Close Planner"));
    titleLayout->addWidget(close);
    connect(close,&Button::clicked,this,&QWidget::hide);
    root->addWidget(titleBar);

    // Keep the three Planner areas separate so they can be developed one at a time.
    auto *firstZone=new Frame(this);
    firstZone->setObjectName("PlannerZoneOne");
    firstZone->setFixedHeight(30);
    root->addWidget(firstZone);

    auto *secondZone=new Frame(this);
    secondZone->setObjectName("PlannerZoneTwo");
    secondZone->setFixedHeight(45);
    root->addWidget(secondZone);

    auto *contentZone=new Frame(this);
    contentZone->setObjectName("PlannerZoneThree");
    auto *contentLayout=new QVBoxLayout(contentZone);
    contentLayout->setContentsMargins(0,0,0,0);
    contentLayout->setSpacing(0);

    auto *listsSplitter=new QSplitter(Qt::Horizontal,contentZone);
    listsSplitter->setObjectName("PlannerListsSplitter");
    listsSplitter->setChildrenCollapsible(false);
    listsSplitter->setHandleWidth(1);

    auto *fallbackPanel=new Frame(listsSplitter);
    fallbackPanel->setObjectName("PlannerFallbackPanel");
    auto *fallbackLayout=new QVBoxLayout(fallbackPanel);
    fallbackLayout->setContentsMargins(0,0,0,0);
    fallbackLayout->setSpacing(0);

    auto *fallbackBar=new Frame(fallbackPanel);
    fallbackBar->setObjectName("framebarra");
    fallbackBar->setFixedHeight(25);
    auto *fallbackBarLayout=new QHBoxLayout(fallbackBar);
    fallbackBarLayout->setContentsMargins(0,0,0,0);
    fallbackBarLayout->setSpacing(0);
    auto *fallbackTitle=new Label(fallbackBar);
    fallbackTitle->setObjectName("PanelTitle");
    fallbackTitle->setText(tr("Standby"));
    fallbackBarLayout->addWidget(fallbackTitle);
    fallbackLayout->addWidget(fallbackBar);

    m_fallbackContents=new PlannerContents(fallbackPanel);
    m_fallbackContents->setObjectName("PlannerFallbackContents");
    fallbackLayout->addWidget(m_fallbackContents,1);
    listsSplitter->addWidget(fallbackPanel);

    auto *weekTabs=new PlannerWeekTabs(contentZone,m_dayContents);
    weekTabs->setObjectName("PlannerWeekTabs");
    m_weekTabs=weekTabs;
    listsSplitter->addWidget(weekTabs);
    listsSplitter->setStretchFactor(0,0);
    listsSplitter->setStretchFactor(1,1);
    listsSplitter->setSizes({220,700});

    contentLayout->addWidget(listsSplitter);
    root->addWidget(contentZone,1);
}
