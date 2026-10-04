#ifndef CAPTURE_H
#define CAPTURE_H

#include "widgets/frame.h"

class Button;
class Label;
class QComboBox;
class QLabel;
class QSlider;
class VuMeter;

class Capture : public Frame
{
    Q_OBJECT

private:
    QComboBox *m_inputDevice = nullptr;
    VuMeter *m_inputMeter = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QLabel *m_volumeValue = nullptr;
    Button *m_recButton = nullptr;
    Button *m_stopButton = nullptr;
    Label *m_recordingTime = nullptr;

public:
    explicit Capture(QWidget *parent = nullptr);
    ~Capture() override;

protected:
    void showEvent(QShowEvent *event) override;

private slots:

public slots:
    void updateInputDevices();

signals:
};

#endif // CAPTURE_H
