#ifndef PLANNERCONTENTS_H
#define PLANNERCONTENTS_H

#include "widgets/ContentsPlayer.h"

class PlannerContents final : public ContentsPlayer
{
public:
    explicit PlannerContents(QWidget *parent = nullptr);
    AudioItemMaxi *createItem(AudioItemMaxi *item) override;

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // PLANNERCONTENTS_H
