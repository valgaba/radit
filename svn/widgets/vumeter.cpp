#include "widgets/vumeter.h"
#include "widgets/levelmeter.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

VuMeter::VuMeter(QWidget *parent) : QWidget(parent)
{
    setObjectName("VuMeter");
    setAccessibleName(tr("Stereo level meter"));
    setMinimumWidth(100);
    setFixedHeight(28);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 3, 4, 3);
    layout->setSpacing(2);

    const auto addChannel = [this, layout](const QString &name) {
        auto *row = new QHBoxLayout;
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(4);
        auto *label = new QLabel(name, this);
        label->setFixedSize(12, 10);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        auto *bar = new LevelMeter(this);
        bar->setFixedHeight(8);
        bar->setAccessibleName(name == "L" ? tr("Left channel") : tr("Right channel"));
        row->addWidget(label);
        row->addWidget(bar, 1);
        layout->addLayout(row);
        return bar;
    };

    m_left = addChannel("L");
    m_right = addChannel("R");

    // Si cesan las muestras (pausa, final o desconexión), no congelar el nivel.
    m_idleTimer = new QTimer(this);
    m_idleTimer->setSingleShot(true);
    m_idleTimer->setInterval(250);
    connect(m_idleTimer, &QTimer::timeout, this, &VuMeter::reset);
}

QSize VuMeter::sizeHint() const
{
    return QSize(180, 28);
}

void VuMeter::setDecibelScale(bool enabled)
{
    m_decibelScale = enabled;
    reset();
}

qreal VuMeter::levelValue(float decibels) const
{
    if (!std::isfinite(decibels) || decibels <= -120.0f)
        return 0;
    // La captura necesita mostrar niveles de micrófono inferiores a los de un
    // archivo normalizado. Cambiar la escala visual no modifica las muestras.
    if (m_decibelScale)
        return std::clamp((decibels + 60.0) / 60.0, 0.0, 1.0);
    // Amplitud lineal: -6.02 dBFS ocupa media barra y 0 dBFS la llena.
    // Los niveles ya incluyen el volumen del canal en MediaManager.
    if (decibels >= 0.0f)
        return 1;
    return std::pow(10.0, decibels / 20.0);
}

void VuMeter::setLevels(float leftDb, float rightDb)
{
    m_left->levelChanged(levelValue(leftDb));
    m_right->levelChanged(levelValue(rightDb));
    m_idleTimer->start();
}

void VuMeter::reset()
{
    m_idleTimer->stop();
    m_left->levelChanged(0);
    m_right->levelChanged(0);
}
