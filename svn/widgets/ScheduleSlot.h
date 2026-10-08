#ifndef SCHEDULESLOT_H
#define SCHEDULESLOT_H

#include "widgets/frame.h"
#include <QColor>

class Label;
class PlannerContents;
class QScrollArea;
class QEvent;

class ScheduleSlot final : public Frame
{
    Q_OBJECT

public:
    explicit ScheduleSlot(QWidget *parent = nullptr);
    PlannerContents *contents() const { return m_contents; }
    void setTimeText(const QString &time);
    int entryMinute() const { return m_entryMinute; }
    QColor accentColor() const { return m_accentColor; }
    void setEntryTime(int minutesAfterMidnight);
    void setAccentColor(const QColor &color);

signals:
    void closeRequested(ScheduleSlot *slot);
    void entryTimeChanged();
    void focusRequested(ScheduleSlot *slot);
    void copyRequested(ScheduleSlot *slot);
    void cutRequested(ScheduleSlot *slot);
    void propertiesRequested(ScheduleSlot *slot);

private:
    Frame *m_header = nullptr;
    Label *m_time = nullptr;
    PlannerContents *m_contents = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    int m_entryMinute = 0;
    QColor m_accentColor;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // SCHEDULESLOT_H
