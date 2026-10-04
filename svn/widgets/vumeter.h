#ifndef VUMETER_H
#define VUMETER_H

#include <QWidget>

class LevelMeter;
class QTimer;

class VuMeter : public QWidget
{
    Q_OBJECT

public:
    explicit VuMeter(QWidget *parent = nullptr);
    QSize sizeHint() const override;
    void setDecibelScale(bool enabled);

public slots:
    // Niveles en dBFS, como los que entrega AudioFrame.
    void setLevels(float leftDb, float rightDb);
    void reset();

private:
    qreal levelValue(float decibels) const;
    bool m_decibelScale = false;
    LevelMeter *m_left = nullptr;
    LevelMeter *m_right = nullptr;
    QTimer *m_idleTimer = nullptr;
};

#endif // VUMETER_H
