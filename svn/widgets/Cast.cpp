/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/Cast.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/vumeter.h"
#include "core/MediaManager.h"
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QIntValidator>
#include <QVBoxLayout>

namespace {
QString settingsPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath("cast.ini");
}
}

Cast::Cast(QWidget *parent) : Frame(parent)
{
    setObjectName("Cast");
    setMinimumHeight(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_mediaManager = new MediaManager(this);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    auto *bar = new Frame(this);
    bar->setObjectName("framebarra");
    bar->setFixedHeight(25);
    auto *barLayout = new QHBoxLayout(bar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    auto *title = new Label(bar);
    title->setObjectName("PanelTitle");
    title->setText(tr("Cast"));
    barLayout->addWidget(title);
    barLayout->addStretch();
    auto *close = new Button(bar);
    close->SetIcon("Close-hover.svg");
    close->setFixedSize(15, 15);
    close->setIconSize(QSize(15, 15));
    close->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    close->setToolTip(tr("Hide Cast"));
    barLayout->addWidget(close);
    connect(close, &Button::clicked, this, &QWidget::hide);
    root->addWidget(bar);
    auto *content = new QWidget(this);
    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(0);
    root->addWidget(content, 1);

    auto *input = new QHBoxLayout;
    input->setSpacing(12);
    input->addWidget(new QLabel(tr("Input device"), this));
    m_inputDevice = new QComboBox(this);
    m_inputDevice->setObjectName("Combo");
    m_inputDevice->setMinimumContentsLength(12);
    m_inputDevice->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_inputDevice->setFixedHeight(28);
    m_inputDevice->setToolTip(tr("Select an audio input device"));
    input->addWidget(m_inputDevice, 1);
    layout->addLayout(input);
    auto *levels = new QHBoxLayout;
    levels->setSpacing(12);
    m_inputMeter = new VuMeter(this);
    m_inputMeter->setFixedWidth(180);
    m_inputMeter->setDecibelScale(false);
    m_inputMeter->setToolTip(tr("Input level"));
    levels->addWidget(m_inputMeter);
    m_volume = new QSlider(Qt::Horizontal, this);
    m_volume->setObjectName("CastVolumeSlider");
    m_volume->setRange(0, 100);
    m_volume->setValue(100);
    m_volume->setMinimumWidth(80);
    m_volume->setFixedHeight(28);
    m_volume->setToolTip(tr("Input volume"));
    levels->addWidget(m_volume, 1);
    m_volumeValue = new QLabel("100 %", this);
    m_volumeValue->setFixedWidth(45);
    levels->addWidget(m_volumeValue);
    layout->addLayout(levels);
    auto *transport = new QHBoxLayout;
    transport->setSpacing(8);
    m_connect = new Button(this);
    m_connect->setText(tr("Connect"));
    m_connect->setToolTip(tr("Start streaming to Icecast"));
    m_stop = new Button(this);
    QIcon stopIcon(":/icons/Stop.svg");
    stopIcon.addFile(":/icons/Stop.svg", QSize(), QIcon::Disabled);
    m_stop->setIcon(stopIcon);
    m_stop->setIconSize(QSize(32, 32));
    m_stop->setFixedSize(50, 24);
    m_stop->setToolTip(tr("Stop streaming"));
    m_stop->setEnabled(false);
    auto *settingsButton = new Button(this);
    settingsButton->setText(tr("Settings"));
    settingsButton->setCheckable(true);
    settingsButton->setToolTip(tr("Icecast server settings"));
    transport->addWidget(m_connect);
    transport->addWidget(m_stop);
    transport->addWidget(settingsButton);
    transport->addStretch();
    m_time = new Label(this);
    m_time->setObjectName("CastTime");
    m_time->setText("00:00:00.00");
    m_time->setWordWrap(false);
    m_time->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont font = m_time->font();
    font.setPointSize(16);
    font.setBold(true);
    m_time->setFont(font);
    transport->addWidget(m_time);
    layout->addLayout(transport);
    m_status = new QLabel(tr("Disconnected"), this);
    m_status->setWordWrap(true);
    m_status->setObjectName("CastStatus");
    layout->addWidget(m_status);

    m_settings = new QWidget(this);
    auto *grid = new QGridLayout(m_settings);
    grid->setContentsMargins(0, 6, 0, 0);
    m_host = new QLineEdit(m_settings);
    m_host->setObjectName("CastHost");
    m_host->setPlaceholderText(tr("Server hostname"));
    m_port = new QLineEdit("8000", m_settings);
    m_port->setValidator(new QIntValidator(1, 65535, m_port));
    m_port->setMaxLength(5);
    m_username = new QLineEdit("source", m_settings);
    m_password = new QLineEdit(m_settings);
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setObjectName("CastPassword");
    m_password->setToolTip(tr("Server source password"));
    m_mount = new QLineEdit("/stream", m_settings);
    m_name = new QLineEdit("Radit", m_settings);
    m_tls = new QCheckBox(tr("TLS"), m_settings);
    m_tls->setToolTip(tr("Use an encrypted connection to a TLS-enabled Icecast server"));
    m_mode = new QComboBox(m_settings);
    m_mode->setObjectName("Combo");
    for (const auto &mode : MediaManager::mp3RecordingModes())
        m_mode->addItem(mode.label, mode.id);
    m_mode->setToolTip(tr("MP3 streaming format"));
    const auto field = [grid, this](int row, int column, const QString &text, QWidget *widget) {
        auto *label = new QLabel(text, m_settings);
        label->setBuddy(widget);
        grid->addWidget(label, row, column);
        grid->addWidget(widget, row, column + 1);
    };
    field(0, 0, tr("Server"), m_host);
    field(0, 2, tr("Port"), m_port);
    field(1, 0, tr("User"), m_username);
    field(1, 2, tr("Password"), m_password);
    field(2, 0, tr("Mount"), m_mount);
    field(2, 2, tr("Name"), m_name);
    grid->addWidget(m_tls, 3, 0);
    grid->addWidget(m_mode, 3, 1, 1, 3);
    layout->addWidget(m_settings);
    m_settings->hide();
    connect(settingsButton, &Button::toggled, m_settings, &QWidget::setVisible);
    layout->addStretch();

    connect(m_mediaManager, &MediaManager::inputLevelsChanged, m_inputMeter, &VuMeter::setLevels);
    connect(m_volume, &QSlider::valueChanged, this, [this](int value) {
        m_volumeValue->setText(QString::number(value) + " %");
        m_mediaManager->setInputVolume(value / 100.0f);
    });
    connect(m_inputDevice, &QComboBox::activated, this, [this]() { monitorInput(); });
    connect(m_mediaManager, &MediaManager::inputError, this, [this](const QString &message) {
        m_connect->setEnabled(false);
        m_inputMeter->reset();
        m_status->setText(message);
    });
    connect(m_connect, &Button::clicked, this, [this, settingsButton]() {
        saveSettings();
        if (m_host->text().trimmed().isEmpty()) {
            settingsButton->setChecked(true);
            m_host->setFocus();
            m_status->setText(tr("Enter the Icecast server settings."));
            return;
        }
        if (!monitorInput()) return;
        CastSettings settings;
        settings.host = m_host->text();
        settings.port = m_port->text().toInt();
        settings.username = m_username->text();
        settings.password = m_password->text();
        settings.mount = m_mount->text().trimmed();
        settings.name = m_name->text();
        settings.mode = m_mode->currentData().toString();
        settings.tls = m_tls->isChecked();
        m_mediaManager->startStreaming(settings);
    });
    connect(m_stop, &Button::clicked, m_mediaManager, &MediaManager::stopStreaming);
    connect(m_mediaManager, &MediaManager::streamingChanged, this, [this](bool active) {
        m_inputDevice->setEnabled(!active && m_inputDevice->currentData().toInt() >= 0);
        m_settings->setEnabled(!active);
        m_connect->setEnabled(!active && m_inputDevice->currentData().toInt() >= 0);
        m_stop->setEnabled(active);
        m_status->setText(active ? tr("Connecting...") : tr("Disconnected"));
        if (!active && isHidden()) m_mediaManager->stopInput();
    });
    connect(m_mediaManager, &MediaManager::streamingConnected, this, [this]() {
        m_status->setText(tr("Streaming — Icecast / MP3"));
    });
    connect(m_mediaManager, &MediaManager::streamingError, m_status, &QLabel::setText);
    connect(m_mediaManager, &MediaManager::streamingTimeChanged, this, [this](qint64 ms) {
        m_time->setText(QString("%1:%2:%3.%4")
            .arg(ms / 3600000, 2, 10, QLatin1Char('0'))
            .arg((ms / 60000) % 60, 2, 10, QLatin1Char('0'))
            .arg((ms / 1000) % 60, 2, 10, QLatin1Char('0'))
            .arg((ms / 10) % 100, 2, 10, QLatin1Char('0')));
    });
    QSettings settings(settingsPath(), QSettings::IniFormat);
    m_host->setText(settings.value("host").toString());
    m_port->setText(QString::number(qBound(1, settings.value("port", 8000).toInt(), 65535)));
    m_username->setText(settings.value("username", "source").toString());
    m_mount->setText(settings.value("mount", "/stream").toString());
    m_name->setText(settings.value("name", "Radit").toString());
    m_tls->setChecked(settings.value("tls", false).toBool());
    int mode = m_mode->findData(settings.value("mode", "mp3-44100-stereo-192").toString());
    m_mode->setCurrentIndex(mode >= 0 ? mode : 1);
    m_volume->setValue(qBound(0, settings.value("volume", 100).toInt(), 100));
    updateInputDevices();
    const QString key = settings.value("inputKey").toString();
    for (const auto &device : MediaManager::inputDevices()) {
        if (!key.isEmpty() && device.key == key)
            m_inputDevice->setCurrentIndex(m_inputDevice->findData(device.id));
    }
}

Cast::~Cast()
{
    saveSettings();
    m_mediaManager->stopInput();
}

void Cast::saveSettings()
{
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.setValue("host", m_host->text());
    if (m_port->hasAcceptableInput())
        settings.setValue("port", m_port->text().toInt());
    settings.setValue("username", m_username->text());
    settings.setValue("mount", m_mount->text());
    settings.setValue("name", m_name->text());
    settings.setValue("mode", m_mode->currentData());
    settings.setValue("tls", m_tls->isChecked());
    settings.setValue("volume", m_volume->value());
    for (const auto &device : MediaManager::inputDevices()) {
        if (device.id == m_inputDevice->currentData().toInt())
            settings.setValue("inputKey", device.key);
    }
}

bool Cast::monitorInput()
{
    const bool ready = m_mediaManager->startInput(m_inputDevice->currentData().toInt());
    m_connect->setEnabled(ready && !m_mediaManager->isStreaming());
    return ready;
}

void Cast::updateInputDevices()
{
    if (m_mediaManager->isStreaming()) return;
    const int previous = m_inputDevice->currentIndex() < 0 ? -1 : m_inputDevice->currentData().toInt();
    const QSignalBlocker blocker(m_inputDevice);
    m_inputDevice->clear();
    int defaultIndex = -1;
    for (const auto &device : MediaManager::inputDevices()) {
        m_inputDevice->addItem(device.name, device.id);
        if (device.isDefault) defaultIndex = m_inputDevice->count() - 1;
    }
    const bool available = m_inputDevice->count() > 0;
    if (available) {
        const int index = m_inputDevice->findData(previous);
        m_inputDevice->setCurrentIndex(index >= 0 ? index : defaultIndex >= 0 ? defaultIndex : 0);
    } else m_inputDevice->addItem(tr("No input devices available"), -1);
    m_inputDevice->setEnabled(available);
    m_connect->setEnabled(available);
    if (isVisible()) monitorInput();
}

void Cast::showEvent(QShowEvent *event)
{
    Frame::showEvent(event);
    updateInputDevices();
}

void Cast::hideEvent(QHideEvent *event)
{
    if (!m_mediaManager->isStreaming()) {
        m_mediaManager->stopInput();
        m_inputMeter->reset();
    }
    Frame::hideEvent(event);
}
