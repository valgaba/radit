#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H


#include <QTimer>
#include <QObject>
#include <QString>
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

signals:
    void positionChanged(double seconds);
    void playbackFinished();
    void vuMeterChanged(float left, float right);
    void audioFrameUpdated(const AudioFrame &frame);

private:

    QTimer* m_timer = nullptr;
    HSTREAM m_stream = 0;

    static void CALLBACK EndSyncCallback(
           HSYNC handle,
           DWORD channel,
           DWORD data,
           void *user
       );

};




#endif // MEDIAMANAGER_H




