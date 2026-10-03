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

public slots:
    // Niveles en dBFS, como los que entrega AudioFrame.
    void setLevels(float leftDb, float rightDb);
    void reset();

private:
    static qreal levelValue(float decibels);
    LevelMeter *m_left = nullptr;
    LevelMeter *m_right = nullptr;
    QTimer *m_idleTimer = nullptr;
};

#endif // VUMETER_H
