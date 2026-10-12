#ifndef AUDIOITEMNETPLANNER_H
#define AUDIOITEMNETPLANNER_H

#include "widgets/AudioItemNetMaxi.h"

class AudioItemNetPlanner final : public AudioItemNetMaxi
{
    Q_OBJECT

public:
    explicit AudioItemNetPlanner(QWidget *parent = nullptr);
    AudioItemNetPlanner(const AudioItemNetMaxi &source, QWidget *parent = nullptr);
    AudioItemMaxi *copy(QWidget *newParent) const override;
    double playbackLimitSeconds() const override { return m_connectionDurationSeconds; }
    void setPlaybackLimitSeconds(double seconds) override
    { setConnectionDurationSeconds(static_cast<int>(seconds+0.5)); }
    int connectionDurationSeconds() const { return m_connectionDurationSeconds; }
    void setConnectionDurationSeconds(int seconds);
    bool editStation() override;

signals:
    void connectionDurationChanged();

private:
    int m_connectionDurationSeconds = 3600;
};

#endif // AUDIOITEMNETPLANNER_H
