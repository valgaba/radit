/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Radit. If not, see <http://www.gnu.org/licenses/>.
*/

#include "widgets/Capture.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/vumeter.h"
#include "core/MediaManager.h"

#include <QComboBox>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

Capture::Capture(QWidget *parent) : Frame(parent)
{
    setObjectName("Capture"); // Para radit.qss.
    setMinimumHeight(135);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    auto *framebarra = new Frame(this);
    framebarra->setObjectName("framebarra");
    framebarra->setFixedHeight(25);
    auto *layoutbarra = new QHBoxLayout(framebarra);
    layoutbarra->setContentsMargins(0, 0, 0, 0);
    layoutbarra->setSpacing(0);
    auto *labeltitle = new Label(framebarra);
    labeltitle->setObjectName("PanelTitle");
    labeltitle->setText(tr("Capture"));
    layoutbarra->addWidget(labeltitle);
    layoutbarra->addStretch(1);
    auto *btnclose = new Button(framebarra);
    btnclose->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    btnclose->setFixedSize(15, 15);
    btnclose->SetIcon("Close-hover.svg");
    btnclose->setIconSize(QSize(15, 15));
    btnclose->setToolTip(tr("Close capture"));
    btnclose->setAccessibleName(btnclose->toolTip());
    layoutbarra->addWidget(btnclose);

    connect(btnclose, &Button::clicked, this, [this]() {
         hide();
    });

    rootLayout->addWidget(framebarra);

    auto *content = new QWidget(this);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(0);
    rootLayout->addWidget(content, 1);
    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(12);

    auto *deviceLabel = new QLabel(tr("Input device"), this);
    m_inputDevice = new QComboBox(this);
    m_inputDevice->setObjectName("Combo");
    m_inputDevice->setMinimumWidth(160);
    m_inputDevice->setFixedHeight(28);
    m_inputDevice->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_inputDevice->setMinimumContentsLength(16);
    m_inputDevice->setToolTip(tr("Select an audio input device"));
    m_inputDevice->setAccessibleName(tr("Input device"));
    deviceLabel->setBuddy(m_inputDevice);
    topRow->addWidget(deviceLabel);
    topRow->addWidget(m_inputDevice, 1);
    layout->addLayout(topRow);

    auto *middleRow = new QHBoxLayout;
    middleRow->setSpacing(12);
    m_inputMeter = new VuMeter(this);
    m_inputMeter->setFixedWidth(180);
    m_inputMeter->setToolTip(tr("Input level"));
    m_inputMeter->reset();
    middleRow->addWidget(m_inputMeter);

    auto *volumeRow = new QHBoxLayout;
    volumeRow->setSpacing(6);
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setObjectName("CaptureVolumeSlider");
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setSingleStep(1);
    m_volumeSlider->setPageStep(10);
    m_volumeSlider->setValue(100);
    m_volumeSlider->setMinimumWidth(100);
    m_volumeSlider->setFixedHeight(28);
    m_volumeSlider->setToolTip(tr("Input volume"));
    m_volumeSlider->setAccessibleName(tr("Input volume"));
    m_volumeValue = new QLabel("100 %", this);
    m_volumeValue->setObjectName("CaptureVolumeValue");
    m_volumeValue->setFixedWidth(45);
    m_volumeValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    volumeRow->addWidget(m_volumeSlider, 1);
    volumeRow->addWidget(m_volumeValue);
    middleRow->addLayout(volumeRow, 1);
    layout->addLayout(middleRow);

    auto *bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(8);
    m_recButton = new Button(this);
    m_recButton->setObjectName("CaptureRecordButton");
    m_recordIcon = QIcon(":/icons/rec.svg");
    m_recordingIcon = QIcon(":/icons/recon.svg");
    // Mantener el color del indicador aunque Rec esté deshabilitado al grabar.
    m_recordIcon.addFile(":/icons/rec.svg", QSize(), QIcon::Disabled);
    m_recordingIcon.addFile(":/icons/recon.svg", QSize(), QIcon::Disabled);
    m_recButton->setIcon(m_recordIcon);
    m_recButton->setIconSize(QSize(40, 50));
    m_recButton->setFixedSize(50, 24);
    m_recButton->setToolTip(tr("Record to MP3"));
    m_recButton->setAccessibleName(tr("Record to MP3"));
    m_stopButton = new Button(this);
    m_stopButton->setObjectName("CaptureStopButton");
    QIcon stopIcon(":/icons/Stop.svg");
    stopIcon.addFile(":/icons/Stop.svg", QSize(), QIcon::Disabled);
    m_stopButton->setIcon(stopIcon);
    m_stopButton->setIconSize(QSize(32, 32));
    m_stopButton->setFixedSize(50, 24);
    m_stopButton->setToolTip(tr("Stop recording"));
    m_stopButton->setAccessibleName(tr("Stop recording"));
    m_recButton->setEnabled(false);
    m_stopButton->setEnabled(false);
    bottomRow->addWidget(m_recButton);
    bottomRow->addWidget(m_stopButton);
    m_recordingMode = new QComboBox(this);
    m_recordingMode->setObjectName("Combo"); // para qss
    m_recordingMode->setFixedHeight(24);
    m_recordingMode->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_recordingMode->setMinimumContentsLength(8);
    m_recordingMode->setMinimumWidth(120);
    m_recordingMode->setMaximumWidth(190);
    m_recordingMode->setAccessibleName(tr("MP3 recording mode"));
    for (const auto &mode : MediaManager::mp3RecordingModes()) {
        const QString rate = mode.sampleRate == 44100 ? "44.1k" : "48k";
        const QString channels = mode.channels == 2 ? tr("Stereo") : tr("Mono");
        m_recordingMode->addItem(QString("%1 / %2 / %3k").arg(rate, channels)
                                    .arg(mode.bitrateKbps), mode.id);
        m_recordingMode->setItemData(m_recordingMode->count() - 1, mode.label, Qt::ToolTipRole);
    }
    m_recordingMode->setCurrentIndex(m_recordingMode->findData("mp3-44100-stereo-192"));
    m_recordingMode->setToolTip(m_recordingMode->currentData(Qt::ToolTipRole).toString());
    bottomRow->addWidget(m_recordingMode, 1);
    m_recordingTime = new Label(this);
    m_recordingTime->setText("00:00:00.00");
    m_recordingTime->setWordWrap(false);
    m_recordingTime->setObjectName("CaptureRecordingTime");
    QFont font = m_recordingTime->font();
    font.setPointSize(16);
    font.setBold(true);
    m_recordingTime->setFont(font);
    m_recordingTime->setFixedHeight(30);
    m_recordingTime->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_recordingTime->ensurePolished();
    m_recordingTime->setMinimumWidth(m_recordingTime->sizeHint().width());
    m_recordingTime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_recordingTime->setToolTip(tr("Recording time"));
    m_recordingTime->setAccessibleName(tr("Recording time"));
    bottomRow->addWidget(m_recordingTime);
    layout->addLayout(bottomRow);
    layout->addStretch(1); // Todo el espacio vertical sobrante queda debajo.

    m_mediaManager = new MediaManager(this);
    m_recordBlinkTimer = new QTimer(this);
    m_recordBlinkTimer->setInterval(500);
    connect(m_recordBlinkTimer, &QTimer::timeout, this, [this]() {
        m_recordBlinkOn = !m_recordBlinkOn;
        m_recButton->setIcon(m_recordBlinkOn ? m_recordingIcon : m_recordIcon);
    });
    connect(m_mediaManager, &MediaManager::inputLevelsChanged,
            m_inputMeter, &VuMeter::setLevels);
    connect(m_mediaManager, &MediaManager::inputError, this, [this](const QString &message) {
        m_recButton->setEnabled(false);
        m_inputMeter->reset();
        m_inputDevice->setToolTip(message);
        QMessageBox::warning(this, tr("Audio input"), message);
    });
    connect(m_recButton, &Button::clicked, this, [this]() {
        m_mediaManager->startRecording();
    });
    connect(m_stopButton, &Button::clicked, this, [this]() {
        m_mediaManager->stopRecording();
    });
    connect(m_mediaManager, &MediaManager::recordingChanged, this, [this](bool recording) {
        m_recordBlinkTimer->stop();
        m_recordBlinkOn = recording;
        m_recButton->setIcon(recording ? m_recordingIcon : m_recordIcon);
        if (recording)
            m_recordBlinkTimer->start();
        m_inputDevice->setEnabled(!recording && m_inputDevice->currentData().toInt() >= 0);
        m_recButton->setEnabled(!recording && m_inputDevice->currentData().toInt() >= 0);
        m_stopButton->setEnabled(recording);
        m_recordingMode->setEnabled(!recording);
    });
    connect(m_mediaManager, &MediaManager::recordingTimeChanged, this, [this](qint64 ms) {
        m_recordingTime->setText(QString("%1:%2:%3.%4")
            .arg(ms / 3600000, 2, 10, QLatin1Char('0'))
            .arg((ms / 60000) % 60, 2, 10, QLatin1Char('0'))
            .arg((ms / 1000) % 60, 2, 10, QLatin1Char('0'))
            .arg((ms / 10) % 100, 2, 10, QLatin1Char('0')));
    });
    connect(m_mediaManager, &MediaManager::recordingFinished, this, [this](const QString &path) {
        m_recordingTime->setToolTip(tr("Recording saved: %1").arg(path));
    });
    connect(m_mediaManager, &MediaManager::recordingError, this, [this](const QString &message) {
        QMessageBox::warning(this, tr("Recording"), message);
    });
    connect(m_inputDevice, &QComboBox::activated, this, [this]() {
        monitorInput();
        emit configurationChanged();
    });
    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_volumeValue->setText(QString::number(value) + " %");
        m_mediaManager->setInputVolume(value / 100.0f);
        emit configurationChanged();
    });
    connect(m_recordingMode, &QComboBox::activated, this, [this]() {
        if (m_mediaManager->setRecordingMode(m_recordingMode->currentData().toString())) {
            m_recordingMode->setToolTip(m_recordingMode->currentData(Qt::ToolTipRole).toString());
            emit configurationChanged();
        } else {
            const QSignalBlocker blocker(m_recordingMode);
            m_recordingMode->setCurrentIndex(m_recordingMode->findData(m_mediaManager->recordingMode()));
        }
    });
    updateInputDevices();
}

Capture::~Capture()
{
    m_recordBlinkTimer->stop();
    m_mediaManager->stopInput();
}

void Capture::monitorInput()
{
    m_inputMeter->setDecibelScale(false);
    m_inputMeter->setToolTip(tr("Input level"));
    m_inputDevice->setToolTip(tr("Select an audio input device"));
    m_recButton->setEnabled(m_mediaManager->startInput(m_inputDevice->currentData().toInt()));
}

void Capture::updateInputDevices()
{
    if (m_mediaManager->isRecording())
        return;
    const int previousDevice = m_inputDevice->currentIndex() >= 0
        ? m_inputDevice->currentData().toInt() : -1;
    const QSignalBlocker blocker(m_inputDevice);
    m_inputDevice->clear();
    int defaultIndex = -1;
    for (const AudioDevice &device : MediaManager::inputDevices()) {
        m_inputDevice->addItem(device.name, device.id);
        m_inputDevice->setItemData(m_inputDevice->count() - 1,
                                   device.isMicrophone, Qt::UserRole + 1);
        if (device.isDefault)
            defaultIndex = m_inputDevice->count() - 1;
    }
    const bool available = m_inputDevice->count() > 0;
    if (available) {
        const int previousIndex = m_inputDevice->findData(previousDevice);
        m_inputDevice->setCurrentIndex(previousIndex >= 0 ? previousIndex
                                      : defaultIndex >= 0 ? defaultIndex : 0);
    } else {
        m_inputDevice->addItem(tr("No input devices available"), -1);
    }
    m_inputDevice->setEnabled(available);
    if (isVisible())
        monitorInput();
}

void Capture::showEvent(QShowEvent *event)
{
    Frame::showEvent(event);
    updateInputDevices();
}
void Capture::hideEvent(QHideEvent *event)
{
    // Ocultar Capture durante una grabación mantiene Rec activo hasta Stop.
    if (m_mediaManager->isRecording()) {
        Frame::hideEvent(event);
        return;
    }
    m_mediaManager->stopInput();
    m_inputMeter->reset();
    Frame::hideEvent(event);
}

int Capture::inputDevice() const
{
    return m_inputDevice->currentData().toInt();
}

float Capture::inputVolume() const
{
    return m_volumeSlider->value() / 100.0f;
}

void Capture::setInputDevice(int device)
{
    const int index = m_inputDevice->findData(device);
    if (index < 0) return;
    const QSignalBlocker blocker(m_inputDevice);
    m_inputDevice->setCurrentIndex(index);
    if (isVisible()) monitorInput();
}

void Capture::setInputVolume(float volume)
{
    const QSignalBlocker blocker(m_volumeSlider);
    const int value = qBound(0, qRound(volume * 100), 100);
    m_volumeSlider->setValue(value);
    m_volumeValue->setText(QString::number(value) + " %");
    m_mediaManager->setInputVolume(value / 100.0f);
}
QString Capture::recordingMode() const
{
    return m_mediaManager->recordingMode();
}

bool Capture::setRecordingMode(const QString &id)
{
    if (!m_mediaManager->setRecordingMode(id)) return false;
    const QSignalBlocker blocker(m_recordingMode);
    m_recordingMode->setCurrentIndex(m_recordingMode->findData(id));
    m_recordingMode->setToolTip(m_recordingMode->currentData(Qt::ToolTipRole).toString());
    return true;
}