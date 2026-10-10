#ifndef PLANNER_H
#define PLANNER_H

#include <QList>
#include <QHash>
#include <QDate>
#include <QPointer>
#include <QSet>
#include <QElapsedTimer>
#include "widgets/frame.h"

class ContentsPlayer;
class FrameOptionsPlayer;
class Label;
class Player;
class VuMeter;
class Button;
class Slider;
class QTimer;
class ScheduleSlot;
class AudioItemMaxi;
class ContentsBase;
class Frame;

class Planner : public Frame
{
    Q_OBJECT
public:
    explicit Planner(QWidget *parent = nullptr);
    QWidget *weekTabs() const { return m_weekTabs; }
    ContentsPlayer *fallbackContents() const { return m_fallbackContents; }
    const QList<ContentsPlayer *> &dayContents() const { return m_dayContents; }
    int devicePlay() const { return m_devicePlay; }
    int deviceCue() const { return m_deviceCue; }
    float volume() const { return m_volume; }
    void setDevicePlay(int device);
    void setDeviceCue(int device) { m_deviceCue=device; }
    bool setVolume(float volume);
    void prepareForScheduleSlotRemoval(ScheduleSlot *slot);
    void prepareForContentRemoval(ContentsBase *contents, AudioItemMaxi *item);

signals:
    void propertiesRequested();
    void playRequested();
    void configurationChanged();

private:
    QWidget *m_weekTabs = nullptr;
    ContentsPlayer *m_fallbackContents = nullptr;
    Frame *m_standbyBar = nullptr;
    Label *m_standbyTitle = nullptr;
    QList<ContentsPlayer *> m_dayContents;
    FrameOptionsPlayer *m_optionsPanel = nullptr;
    Label *m_playbackNameLabel = nullptr;
    Label *m_timeLabel = nullptr;
    int m_devicePlay = 0;
    int m_deviceCue = 0;
    float m_volume = 1.0f;
    Button *m_playButton = nullptr;
    Slider *m_positionSlider = nullptr;
    VuMeter *m_vumeter = nullptr;
    QTimer *m_standbyBlinkTimer = nullptr;
    Player *m_standbyEngine = nullptr;
    Player *m_scheduleEngineA = nullptr;
    Player *m_scheduleEngineB = nullptr;
    Player *m_activeScheduleEngine = nullptr;
    Player *m_fadeFrom = nullptr;
    Player *m_fadeTo = nullptr;
    QHash<Player *, QString> m_enginePlaybackNames;
    QPointer<ScheduleSlot> m_activeScheduleSlot;
    QPointer<ScheduleSlot> m_pendingScheduleSlot;
    QPointer<ScheduleSlot> m_pendingStartSlot;
    QTimer *m_clockTimer = nullptr;
    QTimer *m_fadeTimer = nullptr;
    QTimer *m_standbyResumeTimer = nullptr;
    QElapsedTimer m_fadeClock;
    QSet<QString> m_triggeredSlots;
    QString m_pendingStandbyListPath;
    QString m_triggeredDate;
    QDate m_lastObservedDate;
    bool m_running = false;
    bool m_userIsSeeking = false;
    bool m_standbyBlinkPhase = false;
    double m_positionDuration = 0.0;
    double m_currentPlaybackPosition = 0.0;
    double m_currentPlaybackDuration = 0.0;
    int m_fadeAction = 0;
    int m_fadeDurationMs = 1800;
    float m_lastStandbyLeft = -120.0f;
    float m_lastStandbyRight = -120.0f;
    float m_lastScheduleLeft = -120.0f;
    float m_lastScheduleRight = -120.0f;
    void startPlayback();
    void stopPlayback();
    void checkSchedule();
    void startScheduleSlot(ScheduleSlot *slot);
    void returnToStandby();
    void resumeStandbyAfterBoundaryCheck();
    void beginCrossfade(Player *from, Player *to, int action);
    void finishCrossfade();
    void updatePlannerMeter();
    void updatePlaybackNameLabel();
    void updateStandbyBarTitle();
    void updateRemainingTimeLabel();
    void updateStandbyBarAppearance();
    bool applyStandbyListChange(const QString &path, QString *error = nullptr);
    Player *positionEngine() const;
};

#endif // PLANNER_H
