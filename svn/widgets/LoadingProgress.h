#ifndef LOADINGPROGRESS_H
#define LOADINGPROGRESS_H

#include <QProgressBar>

// Reusable for waveform decoding, list imports and other loading tasks.
// Call these methods on the GUI thread; total == 0 means unknown duration.
class LoadingProgress : public QProgressBar
{
public:
    explicit LoadingProgress(QWidget *parent = nullptr) : QProgressBar(parent)
    {
        setObjectName("LoadingProgress");
        setFixedHeight(24);
        setTextVisible(true);
        setAlignment(Qt::AlignCenter);
        hide();
    }

    void begin(const QString &message, int total = 0)
    {
        m_message = message;
        setAccessibleName(message);
        setRange(0, qMax(0, total));
        setValue(0);
        setFormat(total > 0 ? message + " %p%" : message);
        show();
    }

    void setProgress(int completed, int total = 100)
    {
        if (total <= 0) return;
        setRange(0, total);
        setFormat(m_message + " %p%");
        setValue(qBound(0, completed, total));
    }

    void finish()
    {
        hide();
    }

private:
    QString m_message;
};

#endif
