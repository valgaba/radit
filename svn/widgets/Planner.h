#ifndef PLANNER_H
#define PLANNER_H

#include <QList>
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

signals:
    void propertiesRequested();
    void playRequested();
    void configurationChanged();

private:
    QWidget *m_weekTabs = nullptr;
    ContentsPlayer *m_fallbackContents = nullptr;
    QList<ContentsPlayer *> m_dayContents;
    FrameOptionsPlayer *m_optionsPanel = nullptr;
    Label *m_timeLabel = nullptr;
    int m_devicePlay = 0;
    int m_deviceCue = 0;
    float m_volume = 1.0f;
    Button *m_playButton = nullptr;
    Slider *m_positionSlider = nullptr;
    VuMeter *m_vumeter = nullptr;
    Player *m_standbyEngine = nullptr;
    Player *m_scheduleEngineA = nullptr;
    Player *m_scheduleEngineB = nullptr;
    Player *m_activeScheduleEngine = nullptr;
    Player *m_fadeFrom = nullptr;
    Player *m_fadeTo = nullptr;
    QPointer<ScheduleSlot> m_activeScheduleSlot;
    QPointer<ScheduleSlot> m_pendingScheduleSlot;
    QTimer *m_clockTimer = nullptr;
    QTimer *m_fadeTimer = nullptr;
    QElapsedTimer m_fadeClock;
    QSet<QString> m_triggeredSlots;
    QString m_triggeredDate;
    bool m_running = false;
    bool m_userIsSeeking = false;
    double m_positionDuration = 0.0;
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
    void beginCrossfade(Player *from, Player *to, int action);
    void finishCrossfade();
    void updatePlannerMeter();
    Player *positionEngine() const;
};

#endif // PLANNER_H
