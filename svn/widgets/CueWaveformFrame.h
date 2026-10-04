#ifndef CUEWAVEFORMFRAME_H
#define CUEWAVEFORMFRAME_H

#include "widgets/frame.h"
#include "core/MediaManager.h"
#include <QPointer>

class Button;
class Label;
class QSlider;
class QMouseEvent;
class VuMeter;

class CueWaveformFrame : public Frame
{
    Q_OBJECT
public:
    explicit CueWaveformFrame(MediaManager *cue, QWidget *parent = nullptr);
    ~CueWaveformFrame() override;
    void showWaveform(const QString &filePath, QWidget *player);

signals:
    void playPauseRequested();
    void stopRequested();
    void seekRequested(double seconds);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setWindowSeconds(double seconds);
    QRectF plotRect() const;
    void refreshControls();
    void seekAt(qreal x);
    void centerInApplication();

    QPointer<MediaManager> m_cue;
    QPointer<QWidget> m_player;
    QTimer *m_refresh = nullptr;
    Button *m_zoomIn = nullptr;
    Button *m_zoomOut = nullptr;
    Label *m_time = nullptr;
    QSlider *m_position = nullptr;
    VuMeter *m_vumeter = nullptr;
    double m_windowSeconds = 12;
    double m_cursorFraction = 0.25;
    double m_dragStart = 0;
    bool m_dragging = false;
    QString m_filePath;
    QString m_status;
    AudioWaveform m_waveform;
    std::shared_ptr<std::atomic_bool> m_cancel;
};

#endif
