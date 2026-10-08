#ifndef AUDIOITEMMETEOCLOCKPLANNER_H
#define AUDIOITEMMETEOCLOCKPLANNER_H

#include "widgets/AudioItemMeteoClockMaxi.h"

class AudioItemMeteoClockPlanner final : public AudioItemMeteoClockMaxi
{
    Q_OBJECT

public:
    explicit AudioItemMeteoClockPlanner(QWidget *parent = nullptr);
    AudioItemMeteoClockPlanner(const AudioItemMeteoClockMaxi &source, QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
};

#endif // AUDIOITEMMETEOCLOCKPLANNER_H
