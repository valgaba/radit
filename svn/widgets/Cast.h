#ifndef CAST_H
#define CAST_H

#include "widgets/frame.h"

class MediaManager;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QSlider;
class QLabel;
class Button;
class Label;
class VuMeter;

class Cast : public Frame
{
    Q_OBJECT
public:
    explicit Cast(QWidget *parent = nullptr);
    ~Cast() override;
public slots:
    void updateInputDevices();
protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
private:
    bool monitorInput();
    void saveSettings();
    MediaManager *m_mediaManager;
    QComboBox *m_inputDevice;
    VuMeter *m_inputMeter;
    QSlider *m_volume;
    QLabel *m_volumeValue;
    Button *m_connect;
    Button *m_stop;
    Label *m_time;
    QLabel *m_status;
    QWidget *m_settings;
    QLineEdit *m_host;
    QLineEdit *m_port;
    QLineEdit *m_username;
    QLineEdit *m_password;
    QLineEdit *m_mount;
    QLineEdit *m_name;
    QCheckBox *m_tls;
    QComboBox *m_mode;
};
#endif
