#ifndef SCHEDULESLOT_H
#define SCHEDULESLOT_H

#include "widgets/frame.h"

class Label;
class PlannerContents;
class QScrollArea;

class ScheduleSlot final : public Frame
{
    Q_OBJECT

public:
    explicit ScheduleSlot(QWidget *parent = nullptr);
    PlannerContents *contents() const { return m_contents; }
    void setTimeText(const QString &time);

private:
    Label *m_time = nullptr;
    PlannerContents *m_contents = nullptr;
    QScrollArea *m_scrollArea = nullptr;
};

#endif // SCHEDULESLOT_H
