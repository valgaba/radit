#ifndef AUDIOITEMFILEPLANNER_H
#define AUDIOITEMFILEPLANNER_H

#include "widgets/AudioItemFileMaxi.h"

class AudioItemFilePlanner final : public AudioItemFileMaxi
{
    Q_OBJECT

public:
    explicit AudioItemFilePlanner(QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
};

#endif // AUDIOITEMFILEPLANNER_H
