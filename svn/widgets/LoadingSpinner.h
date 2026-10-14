#ifndef LOADINGSPINNER_H
#define LOADINGSPINNER_H

#include <QColor>
#include <QPainter>
#include <QTimer>
#include <QWidget>

// Small indeterminate spinner for compact Radit item rows.
class LoadingSpinner : public QWidget
{
public:
    explicit LoadingSpinner(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName("LoadingSpinner");
        setFixedSize(18, 18);
        setAccessibleName(tr("Loading"));
        m_timer.setInterval(60);
        connect(&m_timer, &QTimer::timeout, this, [this]() {
            m_angle = (m_angle + 24) % 360;
            update();
        });
        hide();
    }

    void start()
    {
        m_angle = 0;
        show();
        m_timer.start();
        update();
    }

    void stop()
    {
        m_timer.stop();
        hide();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        const qreal side = qMin(width(), height()) - 4.0;
        const QRectF arc((width() - side) / 2.0, (height() - side) / 2.0, side, side);
        QPen pen(palette().highlight().color());
        pen.setWidthF(2.0);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);
        painter.drawArc(arc, m_angle * 16, 245 * 16);
    }

private:
    QTimer m_timer;
    int m_angle = 0;
};

#endif // LOADINGSPINNER_H
