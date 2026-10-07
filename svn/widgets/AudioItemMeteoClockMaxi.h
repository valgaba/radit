#ifndef AUDIOITEMMETEOCLOCKMAXI_H
#define AUDIOITEMMETEOCLOCKMAXI_H
#include "widgets/AudioItemMaxi.h"
#include <QTime>
#include <memory>
class LocutionPack;

class AudioItemMeteoClockMaxi : public AudioItemMaxi
{
    Q_OBJECT
public:
    explicit AudioItemMeteoClockMaxi(QWidget *parent = nullptr);
    void setVoicePackPath(const QString &path);
    bool isLoading() const;
    QString error() const { return m_error; }
    QStringList selectedClips() const { return m_clips; }
    static QStringList clipNames(const QTime &time, double temperature, int humidity);
    bool preparePlayback() override;
    bool loadPreparedPlayback(MediaManager *manager) override;
    bool advancesOnLoop() const override { return true; }
    QString playbackName() const override;
    AudioItemMaxi *copy(QWidget *newParent) const override;
signals:
    void voicePackLoaded();
private:
    bool fail(const QString &message);
    void updatePackState();
    std::shared_ptr<LocutionPack> m_pack;
    QStringList m_clips;
    QString m_error, m_announcement;
};
#endif
