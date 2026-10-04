#ifndef CUEWAVEFORMFRAME_H
#define CUEWAVEFORMFRAME_H

#include "widgets/frame.h"
#include "core/MediaManager.h"
#include <QPointer>

class QLabel;
class Button;

class CueWaveformFrame : public Frame
{
    Q_OBJECT
public:
    explicit CueWaveformFrame(MediaManager *cue, QWidget *parent = nullptr);
    ~CueWaveformFrame() override;
    void showWaveform(const QString &filePath, QWidget *player);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setWindowSeconds(double seconds);

    QPointer<MediaManager> m_cue;
    QPointer<QWidget> m_player;
    QLabel *m_title = nullptr;
    QTimer *m_refresh = nullptr;
    Button *m_zoomIn = nullptr;
    Button *m_zoomOut = nullptr;
    double m_windowSeconds = 12;
    QString m_filePath;
    QString m_status;
    AudioWaveform m_waveform;
    std::shared_ptr<std::atomic_bool> m_cancel;
};

#endif
