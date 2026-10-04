#include "widgets/CueWaveformFrame.h"
#include "widgets/button.h"

#include <QFileInfo>
#include <QFutureWatcher>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QScreen>
#include <QtConcurrent/QtConcurrentRun>
#include <algorithm>
#include <cmath>

CueWaveformFrame::CueWaveformFrame(MediaManager *cue, QWidget *parent)
    : Frame(parent), m_cue(cue)
{
    setObjectName("CueWaveformFrame");
    setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle(tr("Cue waveform"));
    setMinimumSize(200, 110);
    resize(400, 170);
    auto *header = new QVBoxLayout(this);
    header->setContentsMargins(8, 4, 8, 4);
    m_title = new QLabel(this);
    m_title->setObjectName("CueWaveformTitle");
    m_title->setFixedHeight(20);
    header->addWidget(m_title);
    header->addStretch();
    auto *zoomBar = new QHBoxLayout;
    zoomBar->setContentsMargins(0, 0, 0, 0);
    zoomBar->setSpacing(3);
    zoomBar->addStretch();
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
    header->addLayout(zoomBar);
    connect(m_zoomIn, &QPushButton::clicked, this, [this]() { setWindowSeconds(m_windowSeconds / 2); });
    connect(m_zoomOut, &QPushButton::clicked, this, [this]() { setWindowSeconds(m_windowSeconds * 2); });
    m_refresh = new QTimer(this);
    m_refresh->setTimerType(Qt::PreciseTimer);
    m_refresh->setInterval(16);
    connect(m_refresh, &QTimer::timeout, this, [this]() { update(); });
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
    if (m_player != player) {
        if (m_player)
            m_player->removeEventFilter(this);
        m_player = player;
        if (m_player)
            m_player->installEventFilter(this);
    }
    if (m_player)
        resize(m_player->width(), height());
    if (m_filePath != filePath) {
        if (m_cancel)
            m_cancel->store(true);
        m_filePath = filePath;
        m_waveform = {};
        m_title->setText(QFileInfo(filePath).fileName());
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
                update();
            }
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run([filePath, cancel]() {
            return MediaManager::readWaveform(filePath, cancel);
        }));
    }
    if (!isVisible() && m_player) {
        const QPoint above = m_player->mapToGlobal(QPoint(0, -height() - 30));
        const QRect bounds = m_player->screen()->availableGeometry();
        move(std::clamp(above.x(), bounds.left(), std::max(bounds.left(), bounds.right() - width() + 1)),
             std::clamp(above.y(), bounds.top(), std::max(bounds.top(), bounds.bottom() - height() + 1)));
    }
    show();
    raise();
}

void CueWaveformFrame::showEvent(QShowEvent *event)
{
    m_refresh->start();
    Frame::showEvent(event);
}

void CueWaveformFrame::hideEvent(QHideEvent *event)
{
    m_refresh->stop();
    Frame::hideEvent(event);
}

bool CueWaveformFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_player && event->type() == QEvent::Resize && isVisible())
        resize(m_player->width(), height());
    return Frame::eventFilter(watched, event);
}

void CueWaveformFrame::paintEvent(QPaintEvent *event)
{
    Frame::paintEvent(event);
    QPainter painter(this);
    const QRectF plot(6, 28, width() - 12, height() - 64);
    if (!m_status.isEmpty()) {
        painter.setPen(QColor("#8a8d96"));
        painter.drawText(plot, Qt::AlignCenter | Qt::TextWordWrap, m_status);
        return;
    }
    if (m_waveform.peaks.isEmpty() || m_waveform.secondsPerPeak <= 0)
        return;
    const double position = m_cue ? m_cue->getPosition() : 0;
    const double windowSeconds = m_windowSeconds;
    const double start = position - windowSeconds * 0.25;
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
    const qreal cursor = plot.left() + plot.width() * 0.25;
    painter.setPen(QPen(QColor("#dfdbd2"), 1));
    painter.drawLine(QPointF(cursor, plot.top()), QPointF(cursor, plot.bottom()));
}
