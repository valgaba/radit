/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/Planner.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"
#include "widgets/ContentsPlayer.h"
#include "widgets/AudioItemFilePlanner.h"
#include "widgets/AudioItemNetPlanner.h"
#include "widgets/AudioItemMeteoClockMaxi.h"
#include "widgets/MeteoClock.h"
#include "widgets/PlannerContents.h"
#include "widgets/ScheduleSlot.h"
#include "widgets/Player.h"
#include "widgets/ScheduleSlotOptionsDialog.h"
#include "widgets/frameoptionsplayer.h"
#include "widgets/menu.h"
#include "widgets/vumeter.h"
#include "widgets/slider.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QMouseEvent>
#include <QToolTip>
#include <QTimer>
#include <QAction>
#include <QIcon>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QShowEvent>
#include <QMessageBox>
#include <QPointer>
#include <QGuiApplication>
#include <QScreen>
#include "widgets/scrollbar.h"
#include <functional>
#include <cmath>
#include <algorithm>

namespace {
QPointer<ScheduleSlot> g_scheduleSlotClipboard;
bool g_scheduleSlotClipboardIsCut = false;

void showPopupBelow(QWidget *anchor, QWidget *popup)
{
    QPoint position=anchor->mapToGlobal(QPoint(0,anchor->height()));
    QScreen *screen=QGuiApplication::screenAt(position);
    if (!screen)
        screen=QGuiApplication::primaryScreen();
    if (screen) {
        const QRect available=screen->availableGeometry();
        if (position.x()+popup->width()>available.right()+1)
            position.setX(available.right()+1-popup->width());
        if (position.x()<available.left())
            position.setX(available.left());
        if (position.y()+popup->height()>available.bottom()+1)
            position.setY(anchor->mapToGlobal(QPoint(0,-popup->height())).y());
        if (position.y()<available.top())
            position.setY(available.top());
    }
    popup->move(position);
    popup->show();
    popup->raise();
}

struct PlannerSlotInterval {
    int startSeconds = 0;
    int endSeconds = 0;
    bool priority = false;
    bool durationKnown = true;
    ScheduleSlot *slot = nullptr;
};

class PlannerTimeRuler final : public QWidget
{
public:
    explicit PlannerTimeRuler(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName("PlannerTimeRuler");
        setFixedWidth(72);
        setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        setMouseTracking(true);
        setContextMenuPolicy(Qt::CustomContextMenu);
    }

    void setPixelsPerHour(int pixelsPerHour)
    {
        setFixedHeight(24*pixelsPerHour+1);
        update();
    }

    void setSlotIntervals(const QList<PlannerSlotInterval> &intervals)
    {
        m_slotIntervals=intervals;
        update();
    }

protected:
    void mouseMoveEvent(QMouseEvent *event) override
    {
        const int usableHeight=qMax(1,height()-1);
        const int minute=qBound(0,qRound(event->position().y()*24.0*60/usableHeight),24*60-1);
        const QTime time(0,0);
        QToolTip::showText(mapToGlobal(event->position().toPoint()),
                           time.addSecs(minute*60).toString("HH:mm"),this);
        QWidget::mouseMoveEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        QToolTip::hideText();
        QWidget::leaveEvent(event);
    }

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.fillRect(rect(),QColor("#181e2c"));

        const int rulerBottom=height()-1;
        const int labelRight=width()-14;
        const QFontMetrics metrics(font());
        painter.setFont(font());

        painter.setPen(QColor("#333b4f"));
        painter.drawLine(width()-1,0,width()-1,rulerBottom);
        for (int quarter=0; quarter<=96; ++quarter) {
            const int y=qMin(rulerBottom,quarter*(height()-1)/96);
            const bool hour=(quarter%4)==0;
            const bool halfHour=(quarter%2)==0 && !hour;
            const int tickWidth=hour ? 10 : (halfHour ? 8 : (quarter%2==0 ? 6 : 3));
            painter.setPen(hour ? QColor("#80A4AE") : QColor("#535b6a"));
            painter.drawLine(width()-tickWidth,y,width()-2,y);
            if (!hour && !halfHour)
                continue;

            const int hourNumber=quarter/4;
            const QString label=QString("%1:%2").arg(hourNumber,2,10,QLatin1Char('0'))
                .arg(halfHour ? "30" : "00");
            int textY=y-metrics.height()/2;
            textY=qBound(0,textY,height()-metrics.height());
            painter.setPen(QColor("#80A4AE"));
            painter.drawText(QRect(3,textY,labelRight-3,metrics.height()),
                             Qt::AlignLeft|Qt::AlignVCenter,label);
        }

        painter.save();
        const int markerX=width()-5;
        QFont endTimeFont=painter.font();
        endTimeFont.setBold(true);
        painter.setFont(endTimeFont);
        for (const auto &interval : m_slotIntervals) {
            QPen durationPen(interval.priority ? QColor("#d13838") : QColor("#80A4AE"));
            durationPen.setWidth(2);
            painter.setPen(durationPen);
            const int startY=qBound(0,static_cast<int>(qint64(interval.startSeconds)*(height()-1)/(24*60*60)),rulerBottom);
            const int endY=qBound(startY,static_cast<int>(qint64(interval.endSeconds)*(height()-1)/(24*60*60)),rulerBottom);
            painter.drawLine(markerX,startY,markerX,endY);
            painter.drawLine(width()-11,startY,width()-2,startY);
            painter.drawLine(width()-11,endY,width()-2,endY);

            const QString endLabel=!interval.durationKnown
                ? QStringLiteral("--:--:--")
                : interval.endSeconds>=24*60*60
                    ? QStringLiteral("24:00:00")
                    : QTime(0,0).addSecs(interval.endSeconds).toString("HH:mm:ss");
            int textY=endY-metrics.height()/2;
            textY=qBound(0,textY,height()-metrics.height());
            const QRect labelRect(3,textY,labelRight-3,metrics.height());
            painter.fillRect(labelRect,QColor("#181e2c"));
            painter.setPen(durationPen.color());
            painter.drawText(labelRect,Qt::AlignLeft|Qt::AlignVCenter,endLabel);
        }
        painter.restore();
    }

private:
    QList<PlannerSlotInterval> m_slotIntervals;
};

class PlannerDayPage final : public QWidget
{
public:
    explicit PlannerDayPage(QWidget *parent, QScrollArea *scrollArea,
                            QList<ContentsPlayer *> &dayContents)
        : QWidget(parent), m_scrollArea(scrollArea), m_dayContents(dayContents)
    {
        setObjectName("PlannerDayPage");
        setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint &position) {
            showContextMenuAt(position,mapToGlobal(position));
        });
    }

    void showContextMenuAt(const QPoint &timelinePosition,const QPoint &globalPosition)
    {
        Menu menu(this);
        menu.setFixedWidth(200);
        QAction *addSlotAction=menu.addAction(QIcon(":/icons/Add.svg"),tr("Add schedule slot"));
        menu.addSeparator();
        QAction *pasteSlotAction=menu.addAction(QIcon(":/icons/ActionPaste.svg"),tr("Paste schedule slot"));
        QAction *chosen=menu.exec(globalPosition);
        if (chosen==pasteSlotAction) {
            pasteScheduleSlot();
            return;
        }
        if (chosen!=addSlotAction)
            return;

        const int clickedMinute=qBound(0,qRound(timelinePosition.y()*60.0/m_pixelsPerHour),24*60-1);
        const QTime suggestedTime(0,0);
        ScheduleSlotOptionsDialog options(suggestedTime.addSecs(clickedMinute*60),window());
        if (options.exec()!=QDialog::Accepted)
            return;

        const QTime time=options.entryTime();
        const int minutesAfterMidnight=time.hour()*60+time.minute();
        ScheduleSlot *slot=addSlot(time.toString("HH:mm"),minutesAfterMidnight);
        m_dayContents.append(slot->contents());
        m_scrollArea->ensureVisible(0,(minutesAfterMidnight*m_pixelsPerHour)/60,0,0);
    }

    ScheduleSlot *addSlot(const QString &timeText, int minutesAfterMidnight)
    {
        auto *slot=new ScheduleSlot(this);
        slot->setTimeText(timeText);
        slot->setEntryTime(minutesAfterMidnight);
        attachSlot(slot);
        updateSlotPositions();
        slot->show();
        return slot;
    }

    bool hasSlotAtMinute(int minute, const ScheduleSlot *except = nullptr) const
    {
        for (const TimedSlot &entry : m_slots) {
            if (entry.widget!=except && entry.widget->entryMinute()==minute)
                return true;
        }
        return false;
    }

private:
    void attachSlot(ScheduleSlot *slot)
    {
        m_slots.append({slot});
        connect(slot,&ScheduleSlot::entryTimeChanged,this,[this]() {
            updateSlotPositions();
        });
        connect(slot,&ScheduleSlot::disabledChanged,this,[this]() {
            updateSlotPositions();
        });
        connect(slot->contents(),&PlannerContents::contentDurationsChanged,this,[this]() {
            updateSlotPositions();
        });
        connect(slot->contents(),&PlannerContents::contentAdded,this,[this,slot]() {
            if (!m_contentAutoFocusEnabled)
                return;
            QPointer<ScheduleSlot> pendingFocus=slot;
            QTimer::singleShot(0,this,[this,pendingFocus]() {
                if (pendingFocus && m_focusSlot)
                    m_focusSlot(pendingFocus);
            });
        });
        connect(slot,&ScheduleSlot::closeRequested,this,[this](ScheduleSlot *closingSlot) {
            const auto answer=QMessageBox::question(
                this,
                tr("Delete schedule slot"),
                tr("Do you want to delete this schedule slot?"),
                QMessageBox::Yes|QMessageBox::No,
                QMessageBox::No);
            if (answer!=QMessageBox::Yes)
                return;

            m_dayContents.removeAll(closingSlot->contents());
            for (qsizetype index=m_slots.size()-1; index>=0; --index) {
                if (m_slots.at(index).widget==closingSlot)
                    m_slots.removeAt(index);
            }
            closingSlot->hide();
            closingSlot->deleteLater();
            updateSlotPositions();
        });
        connect(slot,&ScheduleSlot::copyRequested,this,[this](ScheduleSlot *source) {
            if (g_scheduleSlotClipboard && !g_scheduleSlotClipboardIsCut)
                delete g_scheduleSlotClipboard.data();
            g_scheduleSlotClipboard=cloneSlot(source,nullptr);
            g_scheduleSlotClipboardIsCut=false;
        });
        connect(slot,&ScheduleSlot::cutRequested,this,[this](ScheduleSlot *source) {
            if (g_scheduleSlotClipboard && !g_scheduleSlotClipboardIsCut)
                delete g_scheduleSlotClipboard.data();
            g_scheduleSlotClipboard=source;
            g_scheduleSlotClipboardIsCut=true;
        });
        connect(slot,&ScheduleSlot::propertiesRequested,this,[this](ScheduleSlot *editedSlot) {
            const QTime currentTime(editedSlot->entryMinute()/60,editedSlot->entryMinute()%60);
            ScheduleSlotOptionsDialog options(currentTime,window(),editedSlot->isScheduleDisabled(),true);
            if (options.exec()==QDialog::Accepted) {
                const QTime time=options.entryTime();
                editedSlot->setEntryTime(time.hour()*60+time.minute());
                editedSlot->setScheduleDisabled(options.isDisabled());
            }
        });
        connect(slot,&ScheduleSlot::focusRequested,this,[this](ScheduleSlot *focusedSlot) {
            if (m_focusSlot)
                m_focusSlot(focusedSlot);
        });
    }

public:
    void setIntervalsChangedCallback(std::function<void()> callback)
    {
        m_intervalsChanged=std::move(callback);
    }

    void setFocusSlotCallback(std::function<void(ScheduleSlot *)> callback)
    {
        m_focusSlot=std::move(callback);
    }

    void setContentAutoFocusEnabled(bool enabled)
    {
        m_contentAutoFocusEnabled=enabled;
    }

    void setPixelsPerHour(int pixelsPerHour)
    {
        m_pixelsPerHour=pixelsPerHour;
        setFixedHeight(24*pixelsPerHour+1);
        updateSlotPositions();
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        updateSlotPositions();
    }

    struct TimedSlot {
        ScheduleSlot *widget;
    };

public:
    QList<PlannerSlotInterval> slotIntervals(bool updatePriorityIndicators = true)
    {
        QList<PlannerSlotInterval> intervals;
        for (const TimedSlot &entry : m_slots) {
            if (updatePriorityIndicators)
                entry.widget->setPriority(false);
            if (entry.widget->isScheduleDisabled())
                continue;
            const int startSeconds=entry.widget->entryMinute()*60;
            const bool durationKnown=entry.widget->totalDurationKnown()
                && entry.widget->totalDurationSeconds()>0.0;
            const int durationSeconds=durationKnown
                ? qRound(entry.widget->totalDurationSeconds())
                : qMax(1,qRound(entry.widget->height()*3600.0/m_pixelsPerHour));
            intervals.append({startSeconds,qMin(24*60*60,startSeconds+durationSeconds),
                              false,durationKnown,entry.widget});
        }

        std::sort(intervals.begin(),intervals.end(),[](const PlannerSlotInterval &left,
                                                       const PlannerSlotInterval &right) {
            return left.startSeconds<right.startSeconds;
        });
        int activeIndex=-1;
        for (int index=0; index<intervals.size(); ++index) {
            PlannerSlotInterval &interval=intervals[index];
            if (updatePriorityIndicators)
                interval.slot->setPriority(false);
            if (activeIndex>=0 && intervals.at(activeIndex).endSeconds>interval.startSeconds) {
                interval.priority=true;
                if (updatePriorityIndicators)
                    interval.slot->setPriority(true);
                intervals[activeIndex].endSeconds=interval.startSeconds;
            }
            activeIndex=index;
        }
        return intervals;
    }

private:
    ScheduleSlot *cloneSlot(const ScheduleSlot *source, QWidget *parent)
    {
        auto *copy=new ScheduleSlot(parent);
        copy->setEntryTime(source->entryMinute());
        copy->setAccentColor(source->accentColor());
        copy->setScheduleDisabled(source->isScheduleDisabled());
        QVBoxLayout *sourceLayout=source->contents()->layout;
        for (int index=0; index<sourceLayout->count(); ++index) {
            QWidget *widget=sourceLayout->itemAt(index)->widget();
            auto *audio=qobject_cast<AudioItemMaxi*>(widget);
            if (!audio)
                continue;
            AudioItemMaxi *audioCopy=audio->copy(copy->contents());
            if (audioCopy)
                copy->contents()->createItem(audioCopy);
        }
        return copy;
    }

    void pasteScheduleSlot()
    {
        if (!g_scheduleSlotClipboard) {
            QMessageBox::information(this,tr("Paste schedule slot"),
                                     tr("Copy or cut a schedule slot first."));
            return;
        }

        const int minute=g_scheduleSlotClipboard->entryMinute();
        if (hasSlotAtMinute(minute)) {
            QMessageBox::information(this,tr("Paste schedule slot"),
                                     tr("This day already has a schedule slot at that time."));
            return;
        }

        if (!g_scheduleSlotClipboardIsCut) {
            ScheduleSlot *copy=cloneSlot(g_scheduleSlotClipboard,this);
            attachSlot(copy);
            updateSlotPositions();
            copy->show();
            m_dayContents.append(copy->contents());
            m_scrollArea->ensureVisible(0,(minute*m_pixelsPerHour)/60,0,0);
            if (m_focusSlot)
                m_focusSlot(copy);
            return;
        }

        ScheduleSlot *movingSlot=g_scheduleSlotClipboard;
        auto *sourcePage=dynamic_cast<PlannerDayPage*>(movingSlot->parentWidget());
        if (!sourcePage) {
            g_scheduleSlotClipboard.clear();
            g_scheduleSlotClipboardIsCut=false;
            return;
        }

        for (qsizetype index=sourcePage->m_slots.size()-1; index>=0; --index) {
            if (sourcePage->m_slots.at(index).widget==movingSlot)
                sourcePage->m_slots.removeAt(index);
        }
        QObject::disconnect(movingSlot,nullptr,sourcePage,nullptr);
        QObject::disconnect(movingSlot->contents(),nullptr,sourcePage,nullptr);
        movingSlot->setParent(this);
        attachSlot(movingSlot);
        sourcePage->updateSlotPositions();
        updateSlotPositions();
        movingSlot->show();
        m_scrollArea->ensureVisible(0,(minute*m_pixelsPerHour)/60,0,0);
        if (m_focusSlot)
            m_focusSlot(movingSlot);
        g_scheduleSlotClipboard.clear();
        g_scheduleSlotClipboardIsCut=false;
    }

    void updateSlotPositions()
    {
        struct Placement {
            ScheduleSlot *widget = nullptr;
            int startSeconds = 0;
            int endSeconds = 0;
            int lane = 0;
        };
        QList<Placement> placements;

        for (const TimedSlot &entry : m_slots) {
            double durationSeconds=0.0;
            const bool hasContents=entry.widget->contents()->layout->count()>0;
            bool durationIsKnown=hasContents;
            for (int index=0; durationIsKnown && index<entry.widget->contents()->layout->count(); ++index) {
                QWidget *content=entry.widget->contents()->layout->itemAt(index)->widget();
                double itemDuration=0.0;
                if (auto *audio=qobject_cast<AudioItemFilePlanner*>(content))
                    itemDuration=audio->second();
                else if (auto *radio=qobject_cast<AudioItemNetPlanner*>(content))
                    itemDuration=radio->connectionDurationSeconds();
                if (!std::isfinite(itemDuration) || itemDuration<=0.0) {
                    durationIsKnown=false;
                    break;
                }
                durationSeconds+=itemDuration;
            }
            entry.widget->setTotalDuration(durationSeconds,
                                           hasContents && durationIsKnown && durationSeconds>0.0);
            if (!hasContents || !durationIsKnown || durationSeconds<=0.0)
                durationSeconds=3600.0;
            const int startSeconds=entry.widget->entryMinute()*60;
            durationSeconds=qMin(durationSeconds,double(qMax(1,24*60*60-startSeconds)));
            const int slotHeight=qMax(1,qRound(durationSeconds*m_pixelsPerHour/3600.0));
            entry.widget->setTimelineHeight(slotHeight);
            placements.append({entry.widget,startSeconds,
                               qMin(24*60*60,startSeconds+qMax(1,qRound(durationSeconds))),0});
        }

        std::sort(placements.begin(),placements.end(),[](const Placement &left,const Placement &right) {
            if (left.startSeconds!=right.startSeconds)
                return left.startSeconds<right.startSeconds;
            return left.endSeconds<right.endSeconds;
        });

        int groupStart=0;
        while (groupStart<placements.size()) {
            int groupEnd=groupStart+1;
            int groupEndSeconds=placements.at(groupStart).endSeconds;
            while (groupEnd<placements.size()
                   && placements.at(groupEnd).startSeconds<groupEndSeconds) {
                groupEndSeconds=qMax(groupEndSeconds,placements.at(groupEnd).endSeconds);
                ++groupEnd;
            }

            QList<int> laneEndSeconds;
            for (int index=groupStart; index<groupEnd; ++index) {
                int lane=0;
                while (lane<laneEndSeconds.size()
                       && laneEndSeconds.at(lane)>placements.at(index).startSeconds)
                    ++lane;
                if (lane==laneEndSeconds.size())
                    laneEndSeconds.append(placements.at(index).endSeconds);
                else
                    laneEndSeconds[lane]=placements.at(index).endSeconds;
                placements[index].lane=lane;
            }

            const int laneCount=laneEndSeconds.size();
            for (int index=groupStart; index<groupEnd; ++index) {
                const Placement &placement=placements.at(index);
                const int left=width()*placement.lane/laneCount;
                const int right=width()*(placement.lane+1)/laneCount;
                const int top=(placement.startSeconds*m_pixelsPerHour)/3600;
                placement.widget->setGeometry(left,top,qMax(1,right-left),placement.widget->height());
            }
            groupStart=groupEnd;
        }
        if (m_intervalsChanged)
            m_intervalsChanged();
    }

    QList<TimedSlot> m_slots;
    int m_pixelsPerHour = 100;
    QScrollArea *m_scrollArea = nullptr;
    QList<ContentsPlayer *> &m_dayContents;
    std::function<void()> m_intervalsChanged;
    std::function<void(ScheduleSlot *)> m_focusSlot;
    bool m_contentAutoFocusEnabled = true;
};

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
        m_dayGroup=group;

        m_zoomOut=new Button(this);
        m_zoomOut->setObjectName("PlannerZoomOut");
        m_zoomOut->SetIcon("zoom-out.svg");
        m_zoomOut->setToolTip(tr("Zoom out"));
        m_zoomIn=new Button(this);
        m_zoomIn->setObjectName("PlannerZoomIn");
        m_zoomIn->SetIcon("zoom-in.svg");
        m_zoomIn->setToolTip(tr("Zoom in"));
        for (auto *button : {m_zoomOut,m_zoomIn}) {
            button->setIconSize(QSize(18,18));
            button->setFixedSize(24,24);
            button->setStyleSheet(
                "QPushButton { background-color: #181e2c; }"
                "QPushButton:hover { background-color: #181e2c; }"
                "QPushButton:pressed { background-color: #181e2c; }");
            button->setAccessibleName(button->toolTip());
        }

        m_pages=new QStackedWidget(this);
        m_pages->setObjectName("PlannerDayPages");
        m_timelineCanvas=new QWidget;
        m_timelineCanvas->setObjectName("PlannerTimelineCanvas");
        m_timelineCanvas->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        m_timelineLayout=new QHBoxLayout(m_timelineCanvas);
        m_timelineLayout->setContentsMargins(0,0,0,0);
        m_timelineLayout->setSpacing(0);
        m_timeRuler=new PlannerTimeRuler(m_timelineCanvas);
        m_timelineLayout->addWidget(m_timeRuler);
        m_timelineLayout->addWidget(m_pages,1);

        m_scrollArea=new QScrollArea(this);
        m_scrollArea->setObjectName("PlannerTimelineScrollArea");
        m_scrollArea->setWidgetResizable(true);
        m_scrollArea->setVerticalScrollBar(new ScrollBar);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_scrollArea->setWidget(m_timelineCanvas);
        connect(m_timeRuler,&QWidget::customContextMenuRequested,this,[this](const QPoint &position) {
            auto *dayPage=static_cast<PlannerDayPage*>(m_pages->currentWidget());
            if (!dayPage)
                return;
            dayPage->showContextMenuAt(position,m_timeRuler->mapToGlobal(position));
        });

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

            auto *dayPage=new PlannerDayPage(m_pages,m_scrollArea,dayContents);
            dayPage->setIntervalsChangedCallback([this,index,dayPage]() {
                if (m_pages->currentIndex()==index)
                    m_timeRuler->setSlotIntervals(dayPage->slotIntervals());
            });
            dayPage->setFocusSlotCallback([this,index,dayPage](ScheduleSlot *slot) {
                if (QAbstractButton *dayButton=m_dayGroup->button(index))
                    dayButton->setChecked(true);
                m_pages->setCurrentIndex(index);
                m_timeRuler->setSlotIntervals(dayPage->slotIntervals());

                const int viewportHeight=qMax(1,m_scrollArea->viewport()->height());
                const double durationMinutes=slot->height()*60.0/qMax(1,m_pixelsPerHour);
                const double targetHeight=viewportHeight*0.72;
                int targetScale=m_pixelsPerHour;
                while (targetScale<1600
                       && durationMinutes*targetScale/60.0<targetHeight) {
                    targetScale=qMin(1600,targetScale*2);
                }
                if (targetScale!=m_pixelsPerHour)
                    setZoom(targetScale,false);

                QTimer::singleShot(0,this,[this,slot]() {
                    const int top=(slot->entryMinute()*m_pixelsPerHour)/60;
                    const int center=top+slot->height()/2;
                    const int desiredScroll=qMax(0,center-m_scrollArea->viewport()->height()/2);
                    m_scrollArea->verticalScrollBar()->setValue(desiredScroll);
                });
            });
            m_pages->addWidget(dayPage);
            connect(button,&QPushButton::clicked,m_pages,[this,index]() {
                m_pages->setCurrentIndex(index);
                m_timeRuler->setSlotIntervals(
                    static_cast<PlannerDayPage*>(m_pages->widget(index))->slotIntervals());
            });
            if (index == 0)
                button->setChecked(true);
        }
        m_timeRuler->setSlotIntervals(
            static_cast<PlannerDayPage*>(m_pages->widget(0))->slotIntervals());

        layout->addLayout(tabRow);
        layout->addWidget(m_scrollArea,1);
        auto *zoomBarWidget=new QWidget(this);
        zoomBarWidget->setObjectName("PlannerZoomBar");
        zoomBarWidget->setAttribute(Qt::WA_StyledBackground,true);
        zoomBarWidget->setStyleSheet("QWidget#PlannerZoomBar { background-color: #181e2c; border: none; }");
        auto *zoomBar=new QHBoxLayout(zoomBarWidget);
        zoomBar->setContentsMargins(0,0,3,3);
        zoomBar->setSpacing(7);
        zoomBar->addStretch(1);
        zoomBar->addWidget(m_zoomOut);
        zoomBar->addWidget(m_zoomIn);
        layout->addWidget(zoomBarWidget);
        setZoom(150,false);
        connect(m_zoomIn,&QPushButton::clicked,this,[this]() {
            if (m_pixelsPerHour<1600) setZoom(m_pixelsPerHour*2,true);
        });
        connect(m_zoomOut,&QPushButton::clicked,this,[this]() {
            if (m_pixelsPerHour>25) setZoom(m_pixelsPerHour/2,true);
        });
    }

    ScheduleSlot *slotAt(const QDateTime &dateTime)
    {
        const int dayIndex=qBound(0,dateTime.date().dayOfWeek()-1,6);
        auto *day=static_cast<PlannerDayPage*>(m_pages->widget(dayIndex));
        if (!day) return nullptr;
        const int secondOfDay=dateTime.time().hour()*3600
            +dateTime.time().minute()*60+dateTime.time().second();
        const auto intervals=day->slotIntervals(false);
        for (const PlannerSlotInterval &interval : intervals) {
            if (secondOfDay>=interval.startSeconds && secondOfDay<interval.endSeconds)
                return interval.slot;
        }
        return nullptr;
    }

    void updateUpcomingSlot(const QDateTime &now, bool playbackActive)
    {
        ScheduleSlot *nextSlot=nullptr;
        int closestSeconds=301;
        const int currentSecond=now.time().hour()*3600
            +now.time().minute()*60+now.time().second();

        for (int dayOffset=0; playbackActive && dayOffset<=1; ++dayOffset) {
            const int dayIndex=qBound(0,now.date().addDays(dayOffset).dayOfWeek()-1,6);
            auto *day=static_cast<PlannerDayPage*>(m_pages->widget(dayIndex));
            if (!day)
                continue;
            const int secondsFromNow=dayOffset*24*60*60-currentSecond;
            for (const PlannerSlotInterval &interval : day->slotIntervals(false)) {
                if (!interval.slot || interval.slot->contents()->layout->count()==0)
                    continue;
                const int untilStart=secondsFromNow+interval.startSeconds;
                if (untilStart<=0 || untilStart>300 || untilStart>=closestSeconds)
                    continue;
                closestSeconds=untilStart;
                nextSlot=interval.slot;
            }
        }

        if (m_upcomingSlot==nextSlot)
            return;
        if (m_upcomingSlot)
            m_upcomingSlot->setUpcoming(false);
        m_upcomingSlot=nextSlot;
        if (m_upcomingSlot)
            m_upcomingSlot->setUpcoming(true);
    }

protected:
    void showEvent(QShowEvent *event) override
    {
        Frame::showEvent(event);
        if (m_initialTimePositioned)
            return;
        m_initialTimePositioned=true;

        const int currentDayIndex=qBound(0,QDate::currentDate().dayOfWeek()-1,6);
        if (QAbstractButton *dayButton=m_dayGroup->button(currentDayIndex))
            dayButton->setChecked(true);
        m_pages->setCurrentIndex(currentDayIndex);
        m_timeRuler->setSlotIntervals(
            static_cast<PlannerDayPage*>(m_pages->widget(currentDayIndex))->slotIntervals());

        const QTime localTime=QTime::currentTime();
        const int currentMinute=localTime.hour()*60+localTime.minute();
        QTimer::singleShot(0,this,[this,currentMinute]() {
            const int currentY=(currentMinute*m_pixelsPerHour)/60;
            const int targetScroll=currentY-m_scrollArea->viewport()->height()/2;
            QScrollBar *scrollBar=m_scrollArea->verticalScrollBar();
            scrollBar->setValue(qBound(0,targetScroll,scrollBar->maximum()));
        });
    }

private:
    void setZoom(int pixelsPerHour, bool preserveScrollPosition)
    {
        const int oldScale=m_pixelsPerHour;
        const int oldScroll=m_scrollArea->verticalScrollBar()->value();
        m_pixelsPerHour=qBound(25,pixelsPerHour,1600);
        const int contentHeight=24*m_pixelsPerHour+1;
        m_timelineCanvas->setFixedHeight(contentHeight);
        m_timeRuler->setPixelsPerHour(m_pixelsPerHour);
        for (int index=0; index<m_pages->count(); ++index) {
            static_cast<PlannerDayPage*>(m_pages->widget(index))->setPixelsPerHour(m_pixelsPerHour);
        }
        m_zoomOut->setEnabled(m_pixelsPerHour>25);
        m_zoomIn->setEnabled(m_pixelsPerHour<1600);

        if (preserveScrollPosition && oldScale>0) {
            const int target=(oldScroll*m_pixelsPerHour)/oldScale;
            QTimer::singleShot(0,this,[this,target]() {
                m_scrollArea->verticalScrollBar()->setValue(target);
            });
        }
    }

    QStackedWidget *m_pages = nullptr;
    QButtonGroup *m_dayGroup = nullptr;
    QWidget *m_timelineCanvas = nullptr;
    QHBoxLayout *m_timelineLayout = nullptr;
    PlannerTimeRuler *m_timeRuler = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    Button *m_zoomIn = nullptr;
    Button *m_zoomOut = nullptr;
    QPointer<ScheduleSlot> m_upcomingSlot;
    int m_pixelsPerHour = 100;
    bool m_initialTimePositioned = false;
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
    auto *topControls=new QHBoxLayout(firstZone);
    topControls->setContentsMargins(4,0,4,0);
    topControls->setSpacing(5);

    auto *vumeter=new VuMeter(firstZone);
    m_vumeter= vumeter;
    vumeter->setFixedWidth(180);
    vumeter->setDecibelScale(false);
    topControls->addWidget(vumeter,0,Qt::AlignVCenter);
    topControls->addStretch(1);

    auto *propertiesButton=new Button(firstZone);
    propertiesButton->setObjectName("PlannerPropertiesButton");
    propertiesButton->SetIcon("Tools.svg");
    propertiesButton->setIconSize(QSize(20,20));
    propertiesButton->setFixedSize(23,23);
    propertiesButton->setToolTip(tr("Planner properties"));
    topControls->addWidget(propertiesButton,0,Qt::AlignVCenter);
    m_optionsPanel=new FrameOptionsPlayer(this);
    connect(propertiesButton,&QPushButton::clicked,this,[this,propertiesButton]() {
        emit propertiesRequested();
        showPopupBelow(propertiesButton,m_optionsPanel);
    });
    root->addWidget(firstZone);

    auto *secondZone=new Frame(this);
    secondZone->setObjectName("PlannerZoneTwo");
    secondZone->setFixedHeight(45);
    auto *centerControls=new QHBoxLayout(secondZone);
    centerControls->setContentsMargins(4,0,4,0);
    centerControls->setSpacing(5);
    auto *playButton=new Button(secondZone);
    m_playButton=playButton;
    playButton->setObjectName("PlannerPlayButton");
    playButton->SetIcon("Play.svg");
    playButton->setIconSize(QSize(30,30));
    playButton->setFixedSize(50,30);
    playButton->setToolTip(tr("Play Planner"));
    centerControls->addWidget(playButton,0,Qt::AlignVCenter);

    m_positionSlider=new Slider(secondZone);
    m_positionSlider->setObjectName("PlannerPositionSlider");
    m_positionSlider->setRange(0,1000);
    m_positionSlider->setEnabled(false);
    m_positionSlider->setFixedHeight(24);
    m_positionSlider->setToolTip(tr("Seek the current Planner audio"));
    m_positionSlider->setAccessibleName(m_positionSlider->toolTip());
    centerControls->addWidget(m_positionSlider,1,Qt::AlignVCenter);
    connect(m_positionSlider,&QSlider::sliderPressed,this,[this]() {
        m_userIsSeeking=true;
    });
    connect(m_positionSlider,&QSlider::sliderReleased,this,[this]() {
        m_userIsSeeking=false;
        Player *engine=positionEngine();
        if (engine && m_positionDuration>0.0)
            engine->seekPlaybackPosition(m_positionDuration*m_positionSlider->value()/1000.0);
    });
    centerControls->addStretch(1);

    m_timeLabel=new Label(secondZone);
    m_timeLabel->setObjectName("PlannerTimeLabel");
    m_timeLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    m_timeLabel->setFixedWidth(125);
    m_timeLabel->setFixedHeight(25);
    QFont timeFont=m_timeLabel->font();
    timeFont.setPointSize(16);
    timeFont.setBold(true);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setText("00:00:00.00");
    centerControls->addWidget(m_timeLabel,0,Qt::AlignVCenter);
    connect(playButton,&QPushButton::clicked,this,[this]() {
        if (m_running) stopPlayback();
        else startPlayback();
        emit playRequested();
    });
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

    // Dedicated playback engines keep Planner audio independent from the three visible players.
    m_standbyEngine=new Player(this);
    m_scheduleEngineA=new Player(this);
    m_scheduleEngineB=new Player(this);
    for (Player *engine : {m_standbyEngine,m_scheduleEngineA,m_scheduleEngineB}) {
        engine->hide();
        engine->setDevicePlay(m_devicePlay);
        engine->setVolume(0.0f);
        connect(engine,&Player::audioLevelsChanged,this,[this,engine](float left,float right) {
            if (engine==m_standbyEngine) {
                m_lastStandbyLeft=left; m_lastStandbyRight=right;
            } else {
                m_lastScheduleLeft=left; m_lastScheduleRight=right;
            }
            updatePlannerMeter();
        });
        connect(engine,&Player::playbackProgressChanged,this,
                [this,engine](double position,double duration,bool seekable) {
            if (engine!=positionEngine()) return;
            m_positionDuration=duration;
            m_positionSlider->setEnabled(m_running && seekable);
            if (m_userIsSeeking)
                return;
            if (seekable && duration>0.0)
                m_positionSlider->setValue(qBound(0,qRound(position*1000.0/duration),1000));
            else
                m_positionSlider->setValue(0);
        });
    }
    connect(m_scheduleEngineA,&Player::sequentialPlaybackFinished,this,[this]() {
        if (m_scheduleEngineA==m_activeScheduleEngine && m_fadeAction!=2) returnToStandby();
    });
    connect(m_scheduleEngineB,&Player::sequentialPlaybackFinished,this,[this]() {
        if (m_scheduleEngineB==m_activeScheduleEngine && m_fadeAction!=2) returnToStandby();
    });

    m_clockTimer=new QTimer(this);
    m_clockTimer->setInterval(250);
    connect(m_clockTimer,&QTimer::timeout,this,[this]() {
        m_timeLabel->setText(QTime::currentTime().toString("HH:mm:ss"));
        if (m_running) checkSchedule();
        static_cast<PlannerWeekTabs*>(m_weekTabs)->updateUpcomingSlot(
            QDateTime::currentDateTime(),m_running);
    });
    m_clockTimer->start();
    m_timeLabel->setText(QTime::currentTime().toString("HH:mm:ss"));

    m_fadeTimer=new QTimer(this);
    m_fadeTimer->setInterval(20);
    m_fadeTimer->setTimerType(Qt::PreciseTimer);
    connect(m_fadeTimer,&QTimer::timeout,this,[this]() {
        const float progress=qBound(0.0f,float(m_fadeClock.elapsed())/m_fadeDurationMs,1.0f);
        const float angle=progress*1.57079632679f;
        const float fadeIn=std::sin(angle);
        const float fadeOut=std::cos(angle);
        if (m_fadeFrom) m_fadeFrom->setVolume(m_volume*fadeOut);
        if (m_fadeTo) m_fadeTo->setVolume(m_volume*fadeIn);
        if (progress>=1.0f) finishCrossfade();
    });
}

bool Planner::setVolume(float volume)
{
    if (!std::isfinite(volume))
        return false;
    m_volume=std::clamp(volume,0.0f,1.0f);
    if (m_fadeTimer && m_fadeTimer->isActive())
        return true;
    if (m_running) {
        if (m_activeScheduleEngine)
            m_activeScheduleEngine->setVolume(m_volume);
        else if (m_standbyEngine)
            m_standbyEngine->setVolume(m_volume);
    }
    return true;
}

Player *Planner::positionEngine() const
{
    if (!m_running)
        return nullptr;
    if (m_fadeTo)
        return m_fadeTo;
    return m_activeScheduleEngine ? m_activeScheduleEngine : m_standbyEngine;
}

void Planner::setDevicePlay(int device)
{
    m_devicePlay=device;
    for (Player *engine : {m_standbyEngine,m_scheduleEngineA,m_scheduleEngineB})
        if (engine) engine->setDevicePlay(device);
}

void Planner::startPlayback()
{
    if (m_running) return;
    m_running=true;
    m_triggeredSlots.clear();
    m_triggeredDate=QDate::currentDate().toString("yyyyMMdd");
    m_clockTimer->start();
    m_playButton->SetIcon("Stop.svg");
    m_playButton->setToolTip(tr("Stop Planner"));
    m_playButton->setAccessibleName(m_playButton->toolTip());
    m_standbyEngine->setDevicePlay(m_devicePlay);
    m_standbyEngine->setVolume(m_volume);
    m_scheduleEngineA->setVolume(0.0f);
    m_scheduleEngineB->setVolume(0.0f);
    m_standbyEngine->startSequentialPlayback(m_fallbackContents,true);
    checkSchedule();
    static_cast<PlannerWeekTabs*>(m_weekTabs)->updateUpcomingSlot(
        QDateTime::currentDateTime(),true);
}

void Planner::stopPlayback()
{
    if (!m_running) return;
    m_fadeTimer->stop();
    m_fadeFrom=nullptr;
    m_fadeTo=nullptr;
    m_fadeAction=0;
    for (Player *engine : {m_standbyEngine,m_scheduleEngineA,m_scheduleEngineB}) {
        engine->stopSequentialPlayback();
        engine->setVolume(0.0f);
    }
    m_activeScheduleEngine=nullptr;
    m_activeScheduleSlot.clear();
    m_pendingScheduleSlot.clear();
    m_running=false;
    m_positionDuration=0.0;
    m_userIsSeeking=false;
    m_positionSlider->setValue(0);
    m_positionSlider->setEnabled(false);
    static_cast<PlannerWeekTabs*>(m_weekTabs)->updateUpcomingSlot(
        QDateTime::currentDateTime(),false);
    m_playButton->SetIcon("Play.svg");
    m_playButton->setToolTip(tr("Play Planner"));
    m_playButton->setAccessibleName(m_playButton->toolTip());
    m_lastStandbyLeft=m_lastStandbyRight=m_lastScheduleLeft=m_lastScheduleRight=-120.0f;
    updatePlannerMeter();
}

void Planner::checkSchedule()
{
    auto *weekTabs=static_cast<PlannerWeekTabs*>(m_weekTabs);
    if (!weekTabs || !m_running) return;
    const QDateTime now=QDateTime::currentDateTime();
    const QString dayKey=now.date().toString("yyyyMMdd");
    if (m_triggeredDate!=dayKey) {
        m_triggeredSlots.clear();
        m_triggeredDate=dayKey;
    }
    ScheduleSlot *slot=weekTabs->slotAt(now);
    if (slot) {
        const QString key=QStringLiteral("%1-%2")
            .arg(dayKey).arg(quintptr(slot),0,16);
        if (!m_triggeredSlots.contains(key)) {
            bool voicePackLoading=false;
            for (int index=0; index<slot->contents()->layout->count(); ++index) {
                auto *item=qobject_cast<AudioItemMeteoClockMaxi*>(
                    slot->contents()->layout->itemAt(index)->widget());
                if (!item)
                    continue;
                if (item->isLoading()) {
                    voicePackLoading=true;
                    break;
                }
                if (!MeteoClock::hasFreshReadings()
                    && MeteoClock::requestCurrentReadings()) {
                    voicePackLoading=true;
                    break;
                }
            }
            if (!voicePackLoading) {
                m_triggeredSlots.insert(key);
                startScheduleSlot(slot);
            }
        }
    }

    // Standby may have been empty when Play was pressed and filled while Planner is running.
    if (!m_activeScheduleEngine && !m_fadeTimer->isActive()
        && !m_standbyEngine->getCurrentItem()) {
        m_standbyEngine->setVolume(m_volume);
        m_standbyEngine->startSequentialPlayback(m_fallbackContents,true);
    }
}

void Planner::startScheduleSlot(ScheduleSlot *slot)
{
    if (!slot || slot->contents()->layout->count()==0)
        return;
    if (m_fadeTimer->isActive()) {
        m_fadeTimer->stop();
        finishCrossfade();
    }
    if (slot==m_activeScheduleSlot || slot==m_pendingScheduleSlot) return;

    Player *incoming = m_activeScheduleEngine==m_scheduleEngineA
        ? m_scheduleEngineB : m_scheduleEngineA;
    incoming->stopSequentialPlayback();
    incoming->setDevicePlay(m_devicePlay);
    incoming->setVolume(0.0f);
    if (!incoming->startSequentialPlayback(slot->contents(),false))
        return;

    Player *outgoing=m_activeScheduleEngine;
    if (!outgoing && m_standbyEngine->getCurrentItem())
        outgoing=m_standbyEngine;
    m_pendingScheduleSlot=slot;
    beginCrossfade(outgoing,incoming,m_activeScheduleEngine ? 2 : 1);
}

void Planner::returnToStandby()
{
    if (!m_running || (m_fadeTimer->isActive() && (m_fadeAction==3 || m_fadeAction==4)))
        return;
    if (m_fadeTimer->isActive()) {
        m_fadeTimer->stop();
        finishCrossfade();
    }
    Player *outgoing=m_activeScheduleEngine;
    m_activeScheduleEngine=nullptr;
    m_activeScheduleSlot.clear();
    m_pendingScheduleSlot.clear();

    m_standbyEngine->setVolume(0.0f);
    if (m_standbyEngine->getCurrentItem())
        m_standbyEngine->stopSequentialPlayback();
    m_standbyEngine->startSequentialPlayback(m_fallbackContents,true);
    beginCrossfade(outgoing,m_standbyEngine,3);
}

void Planner::beginCrossfade(Player *from, Player *to, int action)
{
    if (m_fadeTimer->isActive()) {
        m_fadeTimer->stop();
        finishCrossfade();
    }
    m_fadeFrom=from;
    m_fadeTo=to;
    m_fadeAction=action;
    m_fadeClock.restart();
    m_fadeTimer->start();
    if (!from && to) to->setVolume(0.0f);
    updatePlannerMeter();
}

void Planner::finishCrossfade()
{
    m_fadeTimer->stop();
    Player *from=m_fadeFrom;
    Player *to=m_fadeTo;
    const int action=m_fadeAction;
    m_fadeFrom=nullptr;
    m_fadeTo=nullptr;
    m_fadeAction=0;

    if (action==1 || action==2) {
        if (from==m_standbyEngine && from->getCurrentItem())
            from->stopSequentialPlayback();
        else if (from && from!=to)
            from->stopSequentialPlayback();
        m_activeScheduleEngine=to;
        m_activeScheduleSlot=m_pendingScheduleSlot;
        m_pendingScheduleSlot.clear();
        if (to) to->setVolume(m_volume);
        if (to && !to->getCurrentItem())
            QTimer::singleShot(0,this,[this,to]() {
                if (m_running && m_activeScheduleEngine==to && !to->getCurrentItem())
                    returnToStandby();
            });
    } else if (action==3) {
        if (from && from!=m_standbyEngine)
            from->stopSequentialPlayback();
        m_activeScheduleEngine=nullptr;
        m_activeScheduleSlot.clear();
        if (to) to->setVolume(m_volume);
    }
    updatePlannerMeter();
}

void Planner::updatePlannerMeter()
{
    if (!m_vumeter) return;
    if (m_fadeTimer && m_fadeTimer->isActive()) {
        m_vumeter->setLevels(qMax(m_lastStandbyLeft,m_lastScheduleLeft),
                             qMax(m_lastStandbyRight,m_lastScheduleRight));
    } else if (m_activeScheduleEngine) {
        m_vumeter->setLevels(m_lastScheduleLeft,m_lastScheduleRight);
    } else if (m_running && m_standbyEngine->getCurrentItem()) {
        m_vumeter->setLevels(m_lastStandbyLeft,m_lastStandbyRight);
    } else {
        m_vumeter->reset();
    }
}
