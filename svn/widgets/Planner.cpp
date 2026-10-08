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
#include "widgets/PlannerContents.h"
#include "widgets/ScheduleSlot.h"
#include "widgets/ScheduleSlotOptionsDialog.h"
#include "widgets/menu.h"
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
#include <QMessageBox>
#include <QPointer>
#include "widgets/scrollbar.h"
#include <functional>
#include <cmath>

namespace {
QPointer<ScheduleSlot> g_scheduleSlotClipboard;
bool g_scheduleSlotClipboardIsCut = false;

class PlannerTimeRuler final : public QWidget
{
public:
    explicit PlannerTimeRuler(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName("PlannerTimeRuler");
        setFixedWidth(58);
        setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        setMouseTracking(true);
    }

    void setPixelsPerHour(int pixelsPerHour)
    {
        setFixedHeight(24*pixelsPerHour+1);
        update();
    }

    void setSlotIntervals(const QList<QPair<int,int>> &intervals)
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
        QPen durationPen(QColor("#80A4AE"));
        durationPen.setWidth(2);
        painter.setPen(durationPen);
        const int markerX=width()-5;
        QFont endTimeFont=painter.font();
        endTimeFont.setBold(true);
        painter.setFont(endTimeFont);
        for (const auto &interval : m_slotIntervals) {
            const int startY=qBound(0,interval.first*(height()-1)/(24*60),rulerBottom);
            const int endY=qBound(startY,interval.second*(height()-1)/(24*60),rulerBottom);
            painter.drawLine(markerX,startY,markerX,endY);
            painter.drawLine(width()-11,startY,width()-2,startY);
            painter.drawLine(width()-11,endY,width()-2,endY);

            const QString endLabel=interval.second>=24*60
                ? QStringLiteral("24:00")
                : QTime(0,0).addSecs(interval.second*60).toString("HH:mm");
            int textY=endY-metrics.height()/2;
            textY=qBound(0,textY,height()-metrics.height());
            const QRect labelRect(3,textY,labelRight-3,metrics.height());
            painter.fillRect(labelRect,QColor("#181e2c"));
            painter.setPen(QColor("#80A4AE"));
            painter.drawText(labelRect,Qt::AlignLeft|Qt::AlignVCenter,endLabel);
            painter.setPen(durationPen);
        }
        painter.restore();
    }

private:
    QList<QPair<int,int>> m_slotIntervals;
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
            Menu menu(this);
            menu.setFixedWidth(200);
            QAction *addSlotAction=menu.addAction(QIcon(":/icons/Add.svg"),tr("Add schedule slot"));
            menu.addSeparator();
            QAction *pasteSlotAction=menu.addAction(QIcon(":/icons/ActionPaste.svg"),tr("Paste schedule slot"));
            QAction *chosen=menu.exec(mapToGlobal(position));
            if (chosen==pasteSlotAction) {
                pasteScheduleSlot();
                return;
            }
            if (chosen!=addSlotAction)
                return;

            const int clickedMinute=qBound(0,qRound(position.y()*60.0/m_pixelsPerHour),24*60-1);
            const QTime suggestedTime(0,0);
            ScheduleSlotOptionsDialog options(suggestedTime.addSecs(clickedMinute*60),window());
            if (options.exec()!=QDialog::Accepted)
                return;

            const QTime time=options.entryTime();
            const int minutesAfterMidnight=time.hour()*60+time.minute();
            ScheduleSlot *slot=addSlot(time.toString("HH:mm"),minutesAfterMidnight);
            m_dayContents.append(slot->contents());
            m_scrollArea->ensureVisible(0,(minutesAfterMidnight*m_pixelsPerHour)/60,0,0);
        });
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
        connect(slot->contents(),&PlannerContents::contentDurationsChanged,this,[this]() {
            updateSlotPositions();
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
            ScheduleSlotOptionsDialog options(currentTime,window());
            if (options.exec()==QDialog::Accepted) {
                const QTime time=options.entryTime();
                editedSlot->setEntryTime(time.hour()*60+time.minute());
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
    QList<QPair<int,int>> slotIntervals() const
    {
        QList<QPair<int,int>> intervals;
        for (const TimedSlot &entry : m_slots) {
            const int durationMinutes=qMax(1,qRound(entry.widget->height()*60.0/m_pixelsPerHour));
            const int startMinute=entry.widget->entryMinute();
            intervals.append({startMinute,qMin(24*60,startMinute+durationMinutes)});
        }
        return intervals;
    }

private:
    ScheduleSlot *cloneSlot(const ScheduleSlot *source, QWidget *parent)
    {
        auto *copy=new ScheduleSlot(parent);
        copy->setEntryTime(source->entryMinute());
        copy->setAccentColor(source->accentColor());
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
        g_scheduleSlotClipboard.clear();
        g_scheduleSlotClipboardIsCut=false;
    }

    void updateSlotPositions()
    {
        for (const TimedSlot &entry : m_slots) {
            const int y=(entry.widget->entryMinute()*m_pixelsPerHour)/60;
            double durationSeconds=0.0;
            bool durationIsKnown=entry.widget->contents()->layout->count()>0;
            for (int index=0; durationIsKnown && index<entry.widget->contents()->layout->count(); ++index) {
                QWidget *content=entry.widget->contents()->layout->itemAt(index)->widget();
                auto *audio=qobject_cast<AudioItemFilePlanner*>(content);
                if (!audio || !std::isfinite(audio->second()) || audio->second()<=0.0) {
                    durationIsKnown=false;
                    break;
                }
                durationSeconds+=audio->second();
            }
            const int slotHeight=durationIsKnown
                ? qMax(1,qRound(durationSeconds*m_pixelsPerHour/3600.0))
                : m_pixelsPerHour;
            entry.widget->setFixedHeight(slotHeight);
            entry.widget->setGeometry(0,y,width(),entry.widget->height());
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
        setZoom(100,false);
        connect(m_zoomIn,&QPushButton::clicked,this,[this]() {
            if (m_pixelsPerHour<1600) setZoom(m_pixelsPerHour*2,true);
        });
        connect(m_zoomOut,&QPushButton::clicked,this,[this]() {
            if (m_pixelsPerHour>25) setZoom(m_pixelsPerHour/2,true);
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
    int m_pixelsPerHour = 100;
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
