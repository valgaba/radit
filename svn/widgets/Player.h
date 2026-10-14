#ifndef PLAYER_H
#define PLAYER_H



#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>
#include <QPointer>
#include <QElapsedTimer>

#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/slider.h"
#include "widgets/frame.h"
#include "widgets/TabPlayer.h"
#include "widgets/AudioItemMaxi.h"
#include "core/MediaManager.h"
#include "widgets/frameoptionsplayer.h"

class VuMeter;
class ContentsBase;
class AudioItemFolderMaxi;



class Player: public Frame{


    Q_OBJECT


private:


    QVBoxLayout *layout; //general

   // dividimos en 4 partes
    Frame *framebarra;
    Frame *frametop;
    Frame *framecenter;
    Frame *framedown;
    Frame *frametab;

    QHBoxLayout *layoutbarra;
    QHBoxLayout *layouttop;
    QHBoxLayout *layoutcenter;
    QHBoxLayout *layoutdown;
    QVBoxLayout *layouttab;

    Button * btnoption;

    Button * btnclose;
    Label *labeltitle;

    TabPlayer *tabplayer;
    MediaManager *mediamanager;
    VuMeter *vumeter = nullptr;
    AudioItemMaxi* currentItem = nullptr;
    MediaManager *m_outgoingManager = nullptr;
    MediaManager *m_pendingMixManager = nullptr;
    MediaManager *m_announcementManager = nullptr;
    QPointer<AudioItemMaxi> m_pendingMixItem;
    QPointer<AudioItemMaxi> m_preparingPlaybackItem;
    QPointer<AudioItemMaxi> m_outgoingItem;
    QTimer *m_mixTimer = nullptr;
    QTimer *m_playbackLimitTimer = nullptr;
    QTimer *m_announcementFadeTimer = nullptr;
    QElapsedTimer m_mixClock;
    QElapsedTimer m_playbackLimitClock;
    QElapsedTimer m_announcementFadeClock;
    int m_mixElapsedMs = 0;
    int m_mixDurationMs = 0;
    qint64 m_playbackLimitRemainingMs = 0;
    float m_mixProgress = 1.0f;
    float m_playerVolume = 1.0f;
    float m_announcementFadeStartVolume = 0.08f;
    float m_announcementRecoveryGain = 1.0f;
    float m_announcementMusicGain = 0.08f;
    float m_announcementVoiceGain = 1.0f;
    int m_announcementRecoveryDurationMs = 2500;
    bool m_purgeOutgoing = false;
    bool m_folderIntroAnnounced = false;
    bool m_folderOutroAnnounced = false;
    QString m_queuedAnnouncementPath;
    bool m_mixAttempted = false;
    bool m_pendingMixNeedsPlaybackPreparation = false;
    double m_mixSearchStartPosition = -1.0;
    double m_mixSearchStartLevel = -120.0;
    double m_mixQuietSincePosition = -1.0;
    double m_mixSmoothedLevel = -120.0;
    QPointer<ContentsBase> m_sequenceContents;
    bool m_sequenceRepeat = false;
    bool m_stopSequenceAtTrackEnd = false;
    int m_sequenceIndex = -1;
    QPointer<AudioItemFolderMaxi> m_folderTurnItem;
    int m_folderTurnTracksStarted = 0;
    float m_outgoingLeft = -120.0f, m_outgoingRight = -120.0f;
    void bindMediaManager();
    bool startPreparedItem(AudioItemMaxi *item);
    void checkFolderAnnouncement(const AudioFrame &frame);
    float folderAnnouncementMusicGain() const;
    void startFolderAnnouncement(const QString &audioPath, int voiceVolumePercent = -1,
                                 int musicVolumePercent = -1, double recoverySeconds = -1.0);
    void stopFolderAnnouncement();
    void fadeMusicBackAfterAnnouncement();
    bool tryFolderMix(const AudioFrame &frame);
    void beginMix(AudioItemMaxi *next, MediaManager *incoming, double remaining);
    void finishPendingMix();
    void cancelPendingMix();
    void updateMix();
    void finishMix(bool purge);
    void startPlaybackLimit(double seconds);
    void pausePlaybackLimit();
    void resumePlaybackLimit();
    bool playSequenceFrom(int index);
    void advanceSequence();
    void markFolderTrackStarted(AudioItemMaxi *item);



    Button * btnstop;
    Label *labelnombre;
    Label *labeltiempo;


    Button * btnpause;
    Button * btnrewind;
    Button * btnforward;
    Slider * slider;

    bool m_userIsSeeking = false;
    double m_duration = 0.0;
    QString SecondToTime(double segundos);
    QString m_emissionSourceLabel;
    QString m_emissionEntryId;
    void finishEmission(const QString &result, const QString &details = {});

    int m_deviceplay=0;
    int m_devicecue=0;


    FrameOptionsPlayer *frameoptionsplayer = nullptr; // si no esta a nullptr hace crack

public:

    explicit Player(QWidget *parent = nullptr);
    ~Player();

    void setTitle(QString title);
    QString title() const;
    void setEmissionSourceLabel(const QString &source) { m_emissionSourceLabel=source; }

    void playItem(AudioItemMaxi *item);
    bool startSequentialPlayback(ContentsBase *contents, bool repeat);
    void stopSequentialPlayback();
    bool stopSequentialPlaybackAtTrackEnd();
    void cancelStopSequentialPlaybackAtTrackEnd();
    void pauseMain();
    void stopMain();
    bool seekPlaybackPosition(double seconds);
    bool previewFolderAnnouncement(const QString &audioPath, int voiceVolumePercent,
                                   int musicVolumePercent, double recoverySeconds);
    bool setVolume(float volume);
    float volume() const;
    float committedVolume() const;

    int devicePlay() const;
    void setDevicePlay(int device);

    int deviceCue() const;
    void setDeviceCue(int device);


    AudioItemMaxi* getCurrentItem() const {
        return currentItem;
    }
    ContentsBase *sequentialContents() const;
    int sequentialIndex() const { return m_sequenceIndex; }
    bool isCurrentFolderTrackLastInSequence() const;


    protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


    private slots:


    public slots:


    signals:
    void configurationChanged();
    void sequentialPlaybackFinished();
    void sequentialTrackFinished();
    void audioLevelsChanged(float left, float right);
    void playbackProgressChanged(double position, double duration, bool seekable);
    void currentPlaybackNameChanged(const QString &name);
    void emissionPlaybackStarted(const QString &source, const QString &name, const QString &path);
    void emissionPlaybackFinished(const QString &result, const QString &details);


};





#endif // PLAYER_H
