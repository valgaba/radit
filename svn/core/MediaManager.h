#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H


#include <QTimer>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <memory>
#include <atomic>
#include <QVector>

struct WaveformPeak
{
    float minimum = 0;
    float maximum = 0;
};

struct AudioWaveform
{
    QVector<WaveformPeak> peaks;
    double secondsPerPeak = 0;
    double duration = 0;
    QString error;
};

struct AudioDevice
{
    int id = -1;
    QString name;
    bool isDefault = false;
    bool isMicrophone = false;
    QString key;
};


struct Mp3RecordingMode
{
    QString id;
    QString label;
    int sampleRate;
    int channels;
    int bitrateKbps;
};
struct AudioFrame
{
    double position;
    float left;
    float right;
};

Q_DECLARE_METATYPE(AudioFrame)

struct CastSettings
{
    QString host;
    int port = 8000;
    QString username = "source";
    QString password;
    QString mount = "/stream";
    QString name = "Radit";
    QString mode = "mp3-44100-stereo-192";
    bool tls = false;
};

class MediaManager : public QObject
{
    Q_OBJECT

public:
    explicit MediaManager (QObject *parent = nullptr);
    ~MediaManager ();

    bool initialize();
    void shutdown();

    static QStringList supportedAudioNameFilters();
    static QList<AudioDevice> inputDevices();
    static QList<AudioDevice> outputDevices();
    static AudioWaveform readWaveform(const QString &filePath,
                                      const std::shared_ptr<std::atomic_bool> &cancel,
                                      const std::shared_ptr<std::atomic_int> &progress = nullptr);

    static double readFileDuration(const QString &filePath);
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

    bool startInput(int deviceId);
    void stopInput();
    void setInputVolume(float volume);
    float inputVolume() const;
    static QList<Mp3RecordingMode> mp3RecordingModes();
    bool setRecordingMode(const QString &id);
    QString recordingMode() const;
    QStringList mp3EncodingOptions() const;
    bool startRecording();
    bool stopRecording();
    bool isRecording() const;

    bool startStreaming(const CastSettings &settings);
    void stopStreaming();
    bool isStreaming() const;



signals:
    void streamingChanged(bool active);
    void streamingConnected();
    void streamingTimeChanged(qint64 milliseconds);
    void streamingError(const QString &message);
    void recordingChanged(bool recording);
    void recordingTimeChanged(qint64 milliseconds);
    void recordingFinished(const QString &filePath);
    void recordingError(const QString &message);
    void inputLevelsChanged(float leftDb, float rightDb);
    void inputError(const QString &message);
    void positionChanged(double seconds);
    void playbackFinished();
    void vuMeterChanged(float left, float right);
    void audioFrameUpdated(const AudioFrame &frame);

private:


    QTimer* m_timer = nullptr;
    QTimer* m_inputTimer = nullptr;
    float m_inputVolume = 1.0f;
    QString m_recordingMode = "mp3-44100-stereo-192";
    QTimer* m_deviceRecoveryTimer = nullptr;
    struct Backend;
    std::unique_ptr<Backend> m_backend;
    int m_currentDevice = -1;
    float m_volume = 1.0f;

    bool startDevice(int deviceId);
    void recoverDevice();
    void flushRecordingData();
    void flushStreamingData();


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




