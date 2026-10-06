#ifndef AUDIOITEMNETMAXI_H
#define AUDIOITEMNETMAXI_H

#include "widgets/AudioItemMaxi.h"

class AudioItemNetMaxi : public AudioItemMaxi
{
    Q_OBJECT
public:
    explicit AudioItemNetMaxi(QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
    bool isLiveStream() const override { return true; }
    bool setUrl(const QString &url);
    QString url() const { return filePath(); }
    bool editStation();
};
#endif
