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
    QPointer<AudioItemMaxi> m_pendingMixItem;
    QPointer<AudioItemMaxi> m_outgoingItem;
    QTimer *m_mixTimer = nullptr;
    QTimer *m_playbackLimitTimer = nullptr;
    QElapsedTimer m_mixClock;
    QElapsedTimer m_playbackLimitClock;
    int m_mixElapsedMs = 0;
    int m_mixDurationMs = 0;
    qint64 m_playbackLimitRemainingMs = 0;
    float m_mixProgress = 1.0f;
    float m_playerVolume = 1.0f;
    bool m_purgeOutgoing = false;
    bool m_mixAttempted = false;
    QPointer<ContentsBase> m_sequenceContents;
    bool m_sequenceRepeat = false;
    int m_sequenceIndex = -1;
    float m_outgoingLeft = -120.0f, m_outgoingRight = -120.0f;
    void bindMediaManager();
    bool tryFolderMix(double position);
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

    int m_deviceplay=0;
    int m_devicecue=0;


    FrameOptionsPlayer *frameoptionsplayer = nullptr; // si no esta a nullptr hace crack

public:

    explicit Player(QWidget *parent = nullptr);
    ~Player();

    void setTitle(QString title);
    QString title() const;

    void playItem(AudioItemMaxi *item);
    bool startSequentialPlayback(ContentsBase *contents, bool repeat);
    void stopSequentialPlayback();
    void pauseMain();
    void stopMain();
    bool seekPlaybackPosition(double seconds);
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


    protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;


    private slots:


    public slots:


    signals:
    void configurationChanged();
    void sequentialPlaybackFinished();
    void audioLevelsChanged(float left, float right);
    void playbackProgressChanged(double position, double duration, bool seekable);


};





#endif // PLAYER_H
