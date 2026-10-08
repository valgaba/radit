#ifndef AUDIOITEMFOLDERPLANNER_H
#define AUDIOITEMFOLDERPLANNER_H

#include "widgets/AudioItemFolderMaxi.h"

class AudioItemFolderPlanner final : public AudioItemFolderMaxi
{
    Q_OBJECT

public:
    explicit AudioItemFolderPlanner(QWidget *parent = nullptr);
    AudioItemFolderPlanner(const AudioItemFolderMaxi &source, QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
};

#endif // AUDIOITEMFOLDERPLANNER_H
