#ifndef AUDIOITEMLISTMAXI_H
#define AUDIOITEMLISTMAXI_H

#include "widgets/AudioItemFolderMaxi.h"

class AudioItemListMaxi : public AudioItemFolderMaxi
{
    Q_OBJECT
public:
    explicit AudioItemListMaxi(QWidget *parent = nullptr);
    AudioItemListMaxi(const AudioItemListMaxi &source, QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
};

#endif // AUDIOITEMLISTMAXI_H
