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
#include "bass.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>
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
    m_recButton->setText(tr("Rec"));
    m_recButton->SetIcon("rec.svg");
    m_recButton->setIconSize(QSize(18, 18));
    m_recButton->setFixedSize(80, 30);
    m_recButton->setToolTip(tr("Record to MP3"));
    m_stopButton = new Button(this);
    m_stopButton->setText(tr("Stop"));
    m_stopButton->SetIcon("Stop.svg");
    m_stopButton->setIconSize(QSize(18, 18));
    m_stopButton->setFixedSize(80, 30);
    m_stopButton->setToolTip(tr("Stop recording"));
    // Solo diseño: no simular una grabación que todavía no está implementada.
    m_recButton->setEnabled(false);
    m_stopButton->setEnabled(false);
    bottomRow->addWidget(m_recButton);
    bottomRow->addWidget(m_stopButton);
    bottomRow->addStretch(1);
    m_recordingTime = new QLabel("00:00:00", this);
    m_recordingTime->setObjectName("CaptureRecordingTime");
    m_recordingTime->setMinimumWidth(100);
    m_recordingTime->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_recordingTime->setToolTip(tr("Recording time"));
    m_recordingTime->setAccessibleName(tr("Recording time"));
    bottomRow->addWidget(m_recordingTime);
    layout->addLayout(bottomRow);
    layout->addStretch(1); // Todo el espacio vertical sobrante queda debajo.

    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_volumeValue->setText(QString::number(value) + " %");
    });
    updateInputDevices();
}

Capture::~Capture() {}

void Capture::updateInputDevices()
{
    const int previousDevice = m_inputDevice->currentIndex() >= 0
        ? m_inputDevice->currentData().toInt() : -1;
    const QSignalBlocker blocker(m_inputDevice);
    m_inputDevice->clear();
    int defaultIndex = -1;
    BASS_DEVICEINFO info = {};
    for (DWORD device = 0; BASS_RecordGetDeviceInfo(device, &info); ++device) {
        if (!(info.flags & BASS_DEVICE_ENABLED))
            continue;
        m_inputDevice->addItem(QString::fromUtf8(info.name), static_cast<int>(device));
        if (info.flags & BASS_DEVICE_DEFAULT)
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
}

void Capture::showEvent(QShowEvent *event)
{
    Frame::showEvent(event);
    updateInputDevices();
}
