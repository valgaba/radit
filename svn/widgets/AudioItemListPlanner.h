#ifndef AUDIOITEMLISTPLANNER_H
#define AUDIOITEMLISTPLANNER_H

#include "widgets/AudioItemListMaxi.h"

class AudioItemListPlanner final : public AudioItemListMaxi
{
    Q_OBJECT
public:
    explicit AudioItemListPlanner(QWidget *parent = nullptr);
    AudioItemListPlanner(const AudioItemListMaxi &source, QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
};

#endif // AUDIOITEMLISTPLANNER_H
