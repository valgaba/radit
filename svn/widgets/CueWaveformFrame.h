#ifndef CUEWAVEFORMFRAME_H
#define CUEWAVEFORMFRAME_H

#include "widgets/frame.h"
#include "core/MediaManager.h"
#include <QPointer>

class QLabel;

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
    QPointer<MediaManager> m_cue;
    QPointer<QWidget> m_player;
    QLabel *m_title = nullptr;
    QTimer *m_refresh = nullptr;
    QString m_filePath;
    QString m_status;
    AudioWaveform m_waveform;
    std::shared_ptr<std::atomic_bool> m_cancel;
};

#endif
