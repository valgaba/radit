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
    bool mixEnabled() const override { return m_mixEnabled; }
    double mixSeconds() const override { return m_mixSeconds; }
    void setMixSettings(bool enabled, double seconds) override;
    bool setUrl(const QString &url);
    QString url() const { return filePath(); }
    virtual bool editStation();
private:
    bool m_mixEnabled = false;
    double m_mixSeconds = 3.0;
};
#endif
