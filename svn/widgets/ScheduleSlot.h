#ifndef SCHEDULESLOT_H
#define SCHEDULESLOT_H

#include "widgets/frame.h"
#include <QColor>

class Label;
class Button;
class PlannerContents;
class QScrollArea;
class QEvent;
class QTimer;

class ScheduleSlot final : public Frame
{
    Q_OBJECT

public:
    explicit ScheduleSlot(QWidget *parent = nullptr);
    PlannerContents *contents() const { return m_contents; }
    void setTimeText(const QString &time);
    int entryMinute() const { return m_entryMinute; }
    QColor accentColor() const { return m_accentColor; }
    double totalDurationSeconds() const { return m_totalDurationSeconds; }
    bool totalDurationKnown() const { return m_totalDurationKnown; }
    bool isScheduleDisabled() const { return m_disabled; }
    bool isPriority() const { return m_priority; }
    void setEntryTime(int minutesAfterMidnight);
    void setTotalDuration(double seconds, bool known);
    void setAccentColor(const QColor &color);
    void setTimelineHeight(int height);
    void setScheduleDisabled(bool disabled);
    void setPriority(bool priority);
    void setUpcoming(bool upcoming);

signals:
    void closeRequested(ScheduleSlot *slot);
    void entryTimeChanged();
    void disabledChanged();
    void focusRequested(ScheduleSlot *slot);
    void copyRequested(ScheduleSlot *slot);
    void cutRequested(ScheduleSlot *slot);
    void propertiesRequested(ScheduleSlot *slot);

private:
    Frame *m_header = nullptr;
    Button *m_toggleContents = nullptr;
    Label *m_time = nullptr;
    Label *m_priorityIndicator = nullptr;
    Label *m_duration = nullptr;
    PlannerContents *m_contents = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    int m_entryMinute = 0;
    QColor m_accentColor;
    double m_totalDurationSeconds = 0.0;
    bool m_totalDurationKnown = true;
    bool m_disabled = false;
    bool m_priority = false;
    bool m_upcoming = false;
    bool m_blinkPhase = false;
    bool m_contentsVisible = true;
    int m_timelineHeight = 100;
    QTimer *m_upcomingBlinkTimer = nullptr;

    void updateHeaderAppearance();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SCHEDULESLOT_H
