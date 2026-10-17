#include "widgets/AudioItemListPlanner.h"

AudioItemListPlanner::AudioItemListPlanner(QWidget *parent)
    : AudioItemListMaxi(parent)
{
    setObjectName("AudioItemListPlanner");
    setCompactPresentation(true);
}

AudioItemListPlanner::AudioItemListPlanner(const AudioItemListMaxi &source, QWidget *parent)
    : AudioItemListMaxi(source, parent)
{
    setObjectName("AudioItemListPlanner");
    setCompactPresentation(true);
}

AudioItemMaxi *AudioItemListPlanner::copy(QWidget *newParent) const
{
    return new AudioItemListPlanner(*this, newParent);
}
