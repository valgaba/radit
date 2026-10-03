#ifndef LEVELMETER_H
#define LEVELMETER_H

#include <QColor>
#include <QFrame>
#include <QElapsedTimer>

class QTimer;

class LevelMeter : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QColor barColor READ barColor WRITE setBarColor)
    Q_PROPERTY(QColor peakColor READ peakColor WRITE setPeakColor)

public:
    explicit LevelMeter(QWidget *parent = nullptr);
    QColor barColor() const;
    void setBarColor(const QColor &color);
    QColor peakColor() const;
    void setPeakColor(const QColor &color);

public slots:
    // Nivel normalizado: 0 = vacío, 1 = lleno.
    void levelChanged(qreal level);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void animateLevel();
    qreal m_level = 0.0;
    qreal m_targetLevel = 0.0;
    qreal m_peakLevel = 0.0;
    QTimer *m_animationTimer = nullptr;
    QElapsedTimer m_animationClock;
    QElapsedTimer m_peakClock;
    QColor m_barColor;
    QColor m_peakColor;
};

#endif // LEVELMETER_H
