#ifndef PLANNER_H
#define PLANNER_H

#include <QList>
#include "widgets/frame.h"

class ContentsPlayer;

class Planner : public Frame
{
    Q_OBJECT
public:
    explicit Planner(QWidget *parent = nullptr);
    QWidget *weekTabs() const { return m_weekTabs; }
    ContentsPlayer *fallbackContents() const { return m_fallbackContents; }
    const QList<ContentsPlayer *> &dayContents() const { return m_dayContents; }

private:
    QWidget *m_weekTabs = nullptr;
    ContentsPlayer *m_fallbackContents = nullptr;
    QList<ContentsPlayer *> m_dayContents;
};

#endif // PLANNER_H
