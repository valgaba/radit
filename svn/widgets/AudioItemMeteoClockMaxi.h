#ifndef AUDIOITEMMETEOCLOCKMAXI_H
#define AUDIOITEMMETEOCLOCKMAXI_H
#include "widgets/AudioItemMaxi.h"
#include <QTime>
#include <QPointer>
#include <memory>
class LocutionPack;

class AudioItemMeteoClockMaxi : public AudioItemMaxi
{
    Q_OBJECT
public:
    explicit AudioItemMeteoClockMaxi(QWidget *parent = nullptr);
    void setVoicePackPath(const QString &path);
    bool announcesTime() const { return m_announceTime; }
    bool announcesWeather() const { return m_announceWeather; }
    bool usesElevenLabs() const { return m_useAiVoice; }
    void setUseElevenLabs(bool enabled) { m_useAiVoice = enabled; }
    bool isPreparingPlayback() const override { return m_aiPreparing; }
    void setAnnouncementOptions(bool announceTime, bool announceWeather);
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
    void showOptionsFrame();
    std::shared_ptr<LocutionPack> m_pack;
    QPointer<QWidget> m_optionsFrame;
    QStringList m_clips;
    QString m_error, m_announcement;
    QString m_aiAudioPath;
    bool m_aiPreparing = false;
    bool m_useAiVoice = false;
    bool m_announceTime = true;
    bool m_announceWeather = true;
};
#endif
