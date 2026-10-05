#include "widgets/CueWaveformFrame.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/vumeter.h"

#include <QFutureWatcher>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QScreen>
#include <QSlider>
#include <QSignalBlocker>
#include <QMouseEvent>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>
#include <cmath>

CueWaveformFrame::CueWaveformFrame(MediaManager *cue, QWidget *parent)
    : Frame(parent), m_cue(cue)
{
    setObjectName("CueWaveformFrame");
    setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    setWindowTitle(tr("Cue waveform"));
    setMinimumSize(545, 140);
    setMouseTracking(true);
    resize(640, 260);
    auto *header = new QVBoxLayout(this);
    header->setContentsMargins(8, 6, 8, 6);
    header->setSpacing(4);
    header->addStretch();
    auto *zoomBar = new QHBoxLayout;
    zoomBar->setContentsMargins(0, 0, 0, 0);
    zoomBar->setSpacing(3);
    const QStringList icons = {"playpause.svg", "Stop.svg", "rewind.svg", "forward.svg"};
    const QStringList names = {"CueWaveformPlay", "CueWaveformStop", "CueWaveformRewind", "CueWaveformForward"};
    const QStringList tips = {tr("Play / pause cue"), tr("Stop cue"), tr("Rewind cue"), tr("Forward cue")};
    for (int i = 0; i < icons.size(); ++i) {
        auto *button = new Button(this);
        button->setObjectName(names[i]);
        button->SetIcon(icons[i]);
        button->setIconSize(i < 2 ? QSize(25, 25) : QSize(20, 20));
        button->setFixedSize(23, 23);
        button->setToolTip(tips[i]);
        button->setAccessibleName(tips[i]);
        zoomBar->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, i]() {
            if (i == 0) emit playPauseRequested();
            else if (i == 1) emit stopRequested();
            else emit seekRequested((m_cue ? m_cue->getPosition() : 0) + (i == 2 ? -1 : 1));
            refreshControls();
        });
    }
    m_vumeter = new VuMeter(this);
    m_vumeter->setFixedWidth(160);
    zoomBar->addWidget(m_vumeter);
    auto *btnclose = new Button(this);
    btnclose->setObjectName("CueWaveformClose");
    btnclose->setText(tr("Closed"));
    btnclose->setFixedSize(60, 23);
    btnclose->setToolTip(tr("Close cue waveform"));
    btnclose->setAccessibleName(btnclose->toolTip());
    zoomBar->addWidget(btnclose);
    connect(btnclose, &QPushButton::clicked, this, [this]() { close(); });
    if (m_cue) {
        connect(m_cue, &MediaManager::audioFrameUpdated, this, [this](const AudioFrame &frame) {
            if (isVisible() && m_cue && m_cue->isPlaying())
                m_vumeter->setLevels(frame.left, frame.right);
            else
                m_vumeter->reset();
        });
    }
    m_time = new Label(this);
    m_time->setObjectName("CueWaveformTime");
    m_time->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_time->setFixedWidth(125);
    QFont font = m_time->font();
    font.setPointSize(14);
    m_time->setFont(font);
    m_time->setText("00:00:00.00");
    zoomBar->addStretch();
    zoomBar->addWidget(m_time);
    m_zoomOut = new Button(this);
    m_zoomOut->setObjectName("CueWaveformZoomOut");
    m_zoomOut->SetIcon("zoom-out.svg");
    m_zoomOut->setToolTip(tr("Zoom out"));
    m_zoomIn = new Button(this);
    m_zoomIn->setObjectName("CueWaveformZoomIn");
    m_zoomIn->SetIcon("zoom-in.svg");
    m_zoomIn->setToolTip(tr("Zoom in"));
    for (auto *button : {m_zoomOut, m_zoomIn}) {
        button->setIconSize(QSize(18, 18));
        button->setFixedSize(24, 24);
        button->setAccessibleName(button->toolTip());
        zoomBar->addWidget(button);
    }
    m_position = new QSlider(Qt::Horizontal, this);
    m_position->setObjectName("CueWaveformPosition");
    m_position->setRange(0, 10000);
    m_position->setFixedHeight(18);
    m_position->setToolTip(tr("Cue position"));
    m_position->setAccessibleName(m_position->toolTip());
    header->addWidget(m_position);
    header->addLayout(zoomBar);
    connect(m_position, &QSlider::valueChanged, this, [this](int value) {
        if (m_waveform.duration > 0) {
            emit seekRequested(value * m_waveform.duration / m_position->maximum());
            refreshControls();
        }
    });
    connect(m_zoomIn, &QPushButton::clicked, this, [this]() { setWindowSeconds(m_windowSeconds / 2); });
    connect(m_zoomOut, &QPushButton::clicked, this, [this]() { setWindowSeconds(m_windowSeconds * 2); });
    m_refresh = new QTimer(this);
    m_refresh->setTimerType(Qt::PreciseTimer);
    m_refresh->setInterval(16);
    connect(m_refresh, &QTimer::timeout, this, [this]() { refreshControls(); update(); });
}

QRectF CueWaveformFrame::plotRect() const
{
    return QRectF(6, 28, width() - 12, height() - 90);
}

void CueWaveformFrame::refreshControls()
{
    if (!m_cue || !m_cue->isPlaying())
        m_vumeter->reset();
    const double position = m_cue ? m_cue->getPosition() : 0;
    const qint64 remaining = qint64(std::round(std::max(0.0, m_waveform.duration - position) * 100));
    m_time->setText(QString("%1:%2:%3.%4")
        .arg(remaining / 360000, 2, 10, QLatin1Char('0'))
        .arg((remaining / 6000) % 60, 2, 10, QLatin1Char('0'))
        .arg((remaining / 100) % 60, 2, 10, QLatin1Char('0'))
        .arg(remaining % 100, 2, 10, QLatin1Char('0')));
    m_position->setEnabled(m_waveform.duration > 0);
    if (!m_position->isSliderDown()) {
        const QSignalBlocker block(m_position);
        m_position->setValue(m_waveform.duration > 0
            ? int(position / m_waveform.duration * m_position->maximum()) : 0);
    }
}

void CueWaveformFrame::seekAt(qreal x)
{
    const QRectF plot = plotRect();
    const double fraction = std::clamp((x - plot.left()) / plot.width(), 0.0, 1.0);
    const double seconds = std::clamp(m_dragStart + fraction * m_windowSeconds, 0.0, m_waveform.duration);
    m_cursorFraction = std::clamp((seconds - m_dragStart) / m_windowSeconds, 0.0, 1.0);
    emit seekRequested(seconds);
    refreshControls();
    update();
}

void CueWaveformFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && plotRect().contains(event->position()) && !m_waveform.peaks.isEmpty()) {
        m_dragStart = (m_cue ? m_cue->getPosition() : 0) - m_windowSeconds * m_cursorFraction;
        m_dragging = true;
        seekAt(event->position().x());
        event->accept();
        return;
    }
    Frame::mousePressEvent(event);
}

void CueWaveformFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        seekAt(event->position().x());
        event->accept();
        return;
    }
    const QRectF plot = plotRect();
    if (plot.contains(event->position())) {
        const double cursor = plot.left() + plot.width() * m_cursorFraction;
        setCursor(std::abs(event->position().x() - cursor) < 6 ? Qt::SizeHorCursor : Qt::PointingHandCursor);
    } else {
        unsetCursor();
    }
    Frame::mouseMoveEvent(event);
}

void CueWaveformFrame::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        seekAt(event->position().x());
        m_dragging = false;
        event->accept();
        return;
    }
    Frame::mouseReleaseEvent(event);
}

void CueWaveformFrame::setWindowSeconds(double seconds)
{
    m_windowSeconds = std::clamp(seconds, 1.5, 96.0);
    m_zoomIn->setEnabled(m_windowSeconds > 1.5);
    m_zoomOut->setEnabled(m_windowSeconds < 96);
    update();
}

CueWaveformFrame::~CueWaveformFrame()
{
    if (m_cancel)
        m_cancel->store(true);
}

void CueWaveformFrame::showWaveform(const QString &filePath, QWidget *player)
{
    m_player = player;
    if (m_filePath != filePath) {
        if (m_cancel)
            m_cancel->store(true);
        m_filePath = filePath;
        m_waveform = {};
        m_status = tr("Loading waveform…");
        m_cancel = std::make_shared<std::atomic_bool>(false);
        const auto cancel = m_cancel;
        auto *watcher = new QFutureWatcher<AudioWaveform>(this);
        connect(watcher, &QFutureWatcher<AudioWaveform>::finished, this, [this, watcher, cancel]() {
            if (cancel == m_cancel && !cancel->load()) {
                m_waveform = watcher->result();
                m_status = m_waveform.error;
                if (m_status.isEmpty() && m_waveform.peaks.isEmpty())
                    m_status = tr("No waveform data available.");
                refreshControls();
                update();
            }
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run([filePath, cancel]() {
            return MediaManager::readWaveform(filePath, cancel);
        }));
    }
    show();
    centerInApplication();
    raise();
}

void CueWaveformFrame::centerInApplication()
{
    QWidget *owner = m_player ? m_player->window() : parentWidget() ? parentWidget()->window() : nullptr;
    const QRect bounds = owner ? owner->screen()->availableGeometry() : screen()->availableGeometry();
    const QPoint center = owner ? owner->frameGeometry().center() : bounds.center();
    const QSize outerSize = frameGeometry().size();
    const QPoint position = center - QPoint(outerSize.width() / 2, outerSize.height() / 2);
    move(std::clamp(position.x(), bounds.left(), std::max(bounds.left(), bounds.right() - outerSize.width() + 1)),
         std::clamp(position.y(), bounds.top(), std::max(bounds.top(), bounds.bottom() - outerSize.height() + 1)));
}

void CueWaveformFrame::showEvent(QShowEvent *event)
{
    m_refresh->start();
    Frame::showEvent(event);
}

void CueWaveformFrame::hideEvent(QHideEvent *event)
{
    m_vumeter->reset();
    m_dragging = false;
    m_refresh->stop();
    Frame::hideEvent(event);
}

void CueWaveformFrame::paintEvent(QPaintEvent *event)
{
    Frame::paintEvent(event);
    QPainter painter(this);
    const QRectF plot = plotRect();
    if (!m_status.isEmpty()) {
        painter.setPen(QColor("#8a8d96"));
        painter.drawText(plot, Qt::AlignCenter | Qt::TextWordWrap, m_status);
        return;
    }
    if (m_waveform.peaks.isEmpty() || m_waveform.secondsPerPeak <= 0)
        return;
    const double position = m_cue ? m_cue->getPosition() : 0;
    const double windowSeconds = m_windowSeconds;
    const double start = m_dragging ? m_dragStart : position - windowSeconds * m_cursorFraction;
    // La regla comparte el origen y la escala de la onda, también durante el arrastre.
    painter.save();
    painter.setClipRect(QRectF(plot.left(), 3, plot.width(), 25));
    QFont rulerFont = painter.font();
    rulerFont.setPixelSize(10);
    rulerFont.setBold(false);
    painter.setFont(rulerFont);
    painter.setPen(QColor("#80A4AE"));
    const double wantedStep = windowSeconds * 75 / plot.width();
    const double steps[] = {0.1, 0.2, 0.5, 1, 2, 5, 10, 15, 30, 60, 120};
    double step = steps[sizeof(steps) / sizeof(steps[0]) - 1];
    for (double candidate : steps) {
        if (candidate >= wantedStep) { step = candidate; break; }
    }
    const double minorStep = step / 5;
    const qint64 firstTick = qint64(std::ceil(std::max(0.0, start) / minorStep));
    const double end = std::min(m_waveform.duration, start + windowSeconds);
    painter.drawLine(QPointF(plot.left(), 25), QPointF(plot.right(), 25));
    for (qint64 tick = firstTick; tick * minorStep <= end + 0.000001; ++tick) {
        const double seconds = tick * minorStep;
        const double x = plot.left() + (seconds - start) * plot.width() / windowSeconds;
        const bool major = tick % 5 == 0;
        painter.drawLine(QPointF(x, major ? 20 : 23), QPointF(x, 25));
        if (major) {
            const qint64 tenths = qint64(std::round(seconds * 10));
            QString time = QString("%1:%2")
                .arg((tenths / 600) % 60, 2, 10, QLatin1Char('0'))
                .arg((tenths / 10) % 60, 2, 10, QLatin1Char('0'));
            if (end >= 3600)
                time.prepend(QString("%1:").arg(tenths / 36000, 2, 10, QLatin1Char('0')));
            if (step < 1)
                time += QString(".%1").arg(tenths % 10);
            const qreal textWidth = painter.fontMetrics().horizontalAdvance(time) + 4;
            const qreal left = std::clamp(x - textWidth / 2, plot.left(), plot.right() - textWidth);
            painter.drawText(QRectF(left, 3, textWidth, 16), Qt::AlignCenter, time);
        }
    }
    painter.restore();
    const double half = plot.height() * 0.48;
    const double middle = plot.center().y();
    painter.setClipRect(plot);
    painter.setPen(QColor("#333b4f"));
    painter.drawLine(QPointF(plot.left(), middle), QPointF(plot.right(), middle));
    painter.setPen(QPen(QColor("#80A4AE"), 1));
    for (int x = 0; x < int(plot.width()); ++x) {
        const double firstTime = start + x * windowSeconds / plot.width();
        const double lastTime = start + (x + 1) * windowSeconds / plot.width();
        if (lastTime <= 0 || firstTime >= m_waveform.duration)
            continue;
        const int first = std::max(0, int(std::floor(firstTime / m_waveform.secondsPerPeak)));
        const int last = std::min(int(m_waveform.peaks.size()) - 1,
                                 int(std::floor(lastTime / m_waveform.secondsPerPeak)));
        float low = 0, high = 0;
        for (int i = first; i <= last; ++i) {
            low = std::min(low, m_waveform.peaks[i].minimum);
            high = std::max(high, m_waveform.peaks[i].maximum);
        }
        painter.drawLine(QPointF(plot.left() + x, middle - high * half),
                         QPointF(plot.left() + x, middle - low * half));
    }
    const qreal cursor = plot.left() + plot.width() * m_cursorFraction;
    painter.setPen(QPen(QColor("#dfdbd2"), 1));
    painter.drawLine(QPointF(cursor, plot.top()), QPointF(cursor, plot.bottom()));
}
