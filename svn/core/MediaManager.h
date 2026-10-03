#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H


#include <QTimer>
#include <QObject>
#include <QString>
#include <QStringList>
#include <bass.h>


struct AudioFrame
{
    double position;
    float left;
    float right;
};

Q_DECLARE_METATYPE(AudioFrame)

class MediaManager : public QObject
{
    Q_OBJECT

public:
    explicit MediaManager (QObject *parent = nullptr);
    ~MediaManager ();

    bool initialize();
    void shutdown();

    static QStringList supportedAudioNameFilters();

    double getDurationSecond(const QString &filePath);
    bool loadFile(const QString &filePath);
    void play();
    void pause();
    void stop();

    double getDuration() const;
    double getPosition() const;

    bool isPlaying() const;
    bool isPaused() const;

    void seek(double seconds);
    void seekRelative(double deltaSeconds);
    void rewind();
    void forward();

    bool setDevice(int deviceId);
    int  currentDevice() const;
    void fadeOut(int durationMs);
    bool setVolume(float volume);
    float volume() const;



signals:
    void positionChanged(double seconds);
    void playbackFinished();
    void vuMeterChanged(float left, float right);
    void audioFrameUpdated(const AudioFrame &frame);

private:


    QTimer* m_timer = nullptr;
    QTimer* m_deviceRecoveryTimer = nullptr;
    HSTREAM m_stream = 0;
    int m_currentDevice = -1;
    float m_volume = 1.0f;

    bool startDevice(int deviceId);
    void recoverDevice();


    static void CALLBACK EndSyncCallback(
           HSYNC handle,
           DWORD channel,
           DWORD data,
           void *user
       );


    static void CALLBACK FadeOutSyncCallback(
        HSYNC handle,
        DWORD channel,
        DWORD data,
        void *user
    );


   static void CALLBACK DeviceFailedSyncProc(
           HSYNC handle,
           DWORD channel,
           DWORD data,
           void *user


           );


    float m_silenceThresholdDb = -25.0f; //45
    int   m_silenceDurationMs  = 300; //400


    float m_soundThresholdDb = -40.0f;
    int m_silenceCounter = 0;
    bool m_hadSound = false;

    // margen dinámico
    double m_tailSeconds = 6.0;
    float  m_tailPercent = 0.10f;

   bool shouldStopBySilence(const AudioFrame& frame);

};




#endif // MEDIAMANAGER_H




