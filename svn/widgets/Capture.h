#ifndef CAPTURE_H
#define CAPTURE_H

#include "widgets/frame.h"
#include <QIcon>

class Button;
class Label;
class QComboBox;
class QLabel;
class QSlider;
class VuMeter;
class MediaManager;
class QTimer;

class Capture : public Frame
{
    Q_OBJECT

private:
    MediaManager *m_mediaManager = nullptr;
    QComboBox *m_inputDevice = nullptr;
    QComboBox *m_recordingMode = nullptr;
    VuMeter *m_inputMeter = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QLabel *m_volumeValue = nullptr;
    Button *m_recButton = nullptr;
    Button *m_stopButton = nullptr;
    QTimer *m_recordBlinkTimer = nullptr;
    QIcon m_recordIcon;
    QIcon m_recordingIcon;
    bool m_recordBlinkOn = false;
    Label *m_recordingTime = nullptr;

public:
    explicit Capture(QWidget *parent = nullptr);
    ~Capture() override;
    int inputDevice() const;
    float inputVolume() const;
    QString recordingMode() const;
    bool setRecordingMode(const QString &id);
    void setInputDevice(int device);
    void setInputVolume(float volume);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void monitorInput();

public slots:
    void updateInputDevices();

signals:
    void configurationChanged();
};

#endif // CAPTURE_H
