/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/MeteoClock.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "core/SystemLocation.h"
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QSettings>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

namespace {
MeteoClock::Readings meteoReadings;
QPointer<MeteoClock> meteoClockInstance;
QString settingsPath() { return QDir(QCoreApplication::applicationDirPath()).filePath("meteo.ini"); }
bool validCoordinates(double latitude, double longitude)
{
    return std::isfinite(latitude) && std::isfinite(longitude) && latitude >= -90 && latitude <= 90 && longitude >= -180 && longitude <= 180;
}
}

MeteoClock::MeteoClock(QWidget *parent) : Frame(parent)
{
    meteoClockInstance=this;
    setObjectName("MeteoClock");
    setMinimumSize(210, 180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_weather = new WeatherService(this);
    m_systemLocation = new SystemLocation(this);
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0); root->setSpacing(0);
    auto *bar = new Frame(this); bar->setObjectName("framebarra"); bar->setFixedHeight(25);
    auto *barLayout = new QHBoxLayout(bar); barLayout->setContentsMargins(0, 0, 0, 0); barLayout->setSpacing(0);
    auto *title = new Label(bar); title->setObjectName("PanelTitle"); title->setText(tr("MeteoClock"));
    barLayout->addWidget(title); barLayout->addStretch();
    auto *close = new Button(bar); close->setObjectName("MeteoCloseButton");
    close->SetIcon("Close-hover.svg"); close->setIconSize(QSize(15, 15)); close->setFixedSize(15, 15);
    close->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    close->setToolTip(tr("Close MeteoClock")); close->setAccessibleName(close->toolTip());
    barLayout->addWidget(close); connect(close, &Button::clicked, this, &QWidget::hide);
    root->addWidget(bar);
    auto *content = new QWidget(this); auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(10, 8, 10, 8); layout->setSpacing(6); root->addWidget(content);
    m_time = new QLabel(this); m_time->setObjectName("MeteoTime"); m_time->setAlignment(Qt::AlignCenter);
    m_time->setFixedHeight(54); m_time->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    QFont timeFont=m_time->font(); timeFont.setPointSize(32); timeFont.setBold(true); m_time->setFont(timeFont);
    layout->addWidget(m_time);
    m_date = new QLabel(this); m_date->setObjectName("MeteoDate"); m_date->setAlignment(Qt::AlignCenter); m_date->setWordWrap(true);
    m_date->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred); layout->addWidget(m_date);
    auto *weatherRow = new QHBoxLayout; weatherRow->setSpacing(12);
    const auto weatherColumn = [this, weatherRow](const QString &title, const QString &objectName, const QString &initial) {
        auto *column = new QVBoxLayout; column->setSpacing(1);
        auto *label = new QLabel(title, this); label->setAlignment(Qt::AlignCenter); column->addWidget(label);
        auto *value = new QLabel(initial, this); value->setObjectName(objectName); value->setAlignment(Qt::AlignCenter);
        value->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred); column->addWidget(value);
        weatherRow->addLayout(column, 1); return value;
    };
    m_temperature = weatherColumn(tr("Temperature"), "MeteoTemperature", "-- °C");
    m_humidity = weatherColumn(tr("Humidity"), "MeteoHumidity", "-- %"); layout->addLayout(weatherRow);
    m_location = new QLabel(tr("Choose a location"), this); m_location->setObjectName("MeteoLocation");
    m_location->setAlignment(Qt::AlignCenter); m_location->setWordWrap(true); layout->addWidget(m_location);
    m_status = new QLabel(this); m_status->setObjectName("MeteoStatus"); m_status->setWordWrap(true); m_status->hide(); layout->addWidget(m_status);
    m_settings = new QWidget(this); m_settings->setObjectName("MeteoSettings");
    auto *settingsLayout = new QVBoxLayout(m_settings); settingsLayout->setContentsMargins(0, 4, 0, 0); settingsLayout->setSpacing(6);
    auto *searchRow = new QHBoxLayout; searchRow->setSpacing(6);
    m_city = new QLineEdit(this); m_city->setObjectName("MeteoCity"); m_city->setPlaceholderText(tr("Search city"));
    m_city->setToolTip(tr("Enter a city name and select a location from the results"));
    m_search = new Button(this); m_search->setText(tr("Search")); m_search->setObjectName("MeteoSearch");
    searchRow->addWidget(m_city, 1); searchRow->addWidget(m_search); settingsLayout->addLayout(searchRow);
    m_places = new QComboBox(this); m_places->setObjectName("Combo"); m_places->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_places->setMinimumContentsLength(10); m_places->setEnabled(false); settingsLayout->addWidget(m_places);
    auto *actionRow = new QHBoxLayout; actionRow->setSpacing(6);
    m_autoLocation = new Button(this); m_autoLocation->setText(tr("Use system location"));
    m_autoLocation->setObjectName("MeteoSystemLocation"); m_autoLocation->setToolTip(tr("Use the location provided by Windows"));
    m_apply = new Button(this); m_apply->setText(tr("Apply")); m_apply->setObjectName("MeteoApply"); m_apply->setEnabled(false);
    actionRow->addWidget(m_autoLocation); actionRow->addStretch(); actionRow->addWidget(m_apply); settingsLayout->addLayout(actionRow);
    auto *provider = new QLabel("<a href=\"https://open-meteo.com/\" style=\"color:#80A4AE\">Open-Meteo</a>", this);
    provider->setObjectName("MeteoProvider"); provider->setAlignment(Qt::AlignRight); provider->setOpenExternalLinks(true); layout->addWidget(provider);
    m_options = new Button(this); m_options->setObjectName("MeteoSettingsButton");
    m_options->setText(tr("Settings")); m_options->setCheckable(true); m_options->setToolTip(tr("Location settings"));
    auto *optionsRow = new QHBoxLayout; optionsRow->setContentsMargins(0, 0, 0, 0);
    optionsRow->addWidget(m_options); optionsRow->addStretch(); layout->addLayout(optionsRow);
    layout->addWidget(m_settings);
    root->addStretch();
    connect(m_options, &Button::toggled, m_settings, &QWidget::setVisible);
    connect(m_search, &Button::clicked, this, &MeteoClock::searchCity);
    connect(m_city, &QLineEdit::returnPressed, this, &MeteoClock::searchCity);
    connect(m_city, &QLineEdit::textEdited, this, [this]() {
        m_weather->cancelSearch(); m_results.clear(); m_places->clear(); m_places->setEnabled(false); m_apply->setEnabled(false); m_search->setEnabled(true);
    });
    connect(m_apply, &Button::clicked, this, [this]() {
        const int index = m_places->currentIndex(); if (index < 0 || index >= m_results.size()) return;
        m_systemLocation->cancel(); m_autoLocation->setEnabled(true);
        const auto place = m_results[index]; setLocation(place.name, place.latitude, place.longitude); m_options->setChecked(false);
    });
    connect(m_autoLocation, &Button::clicked, this, [this]() {
        m_autoLocation->setEnabled(false); setStatus(tr("Getting system location...")); m_systemLocation->request();
    });
    connect(m_systemLocation, &SystemLocation::locationReady, this, [this](double latitude, double longitude) {
        m_autoLocation->setEnabled(true); setLocation(tr("System location"), latitude, longitude); m_options->setChecked(false);
    });
    connect(m_systemLocation, &SystemLocation::locationFailed, this, [this](const QString &message) {
        m_autoLocation->setEnabled(true); setStatus(message); m_options->setChecked(true);
    });
    connect(m_weather, &WeatherService::placesReady, this, [this](const QList<WeatherPlace> &places) {
        m_search->setEnabled(true); m_results=places; m_places->clear();
        for (const auto &place:places) m_places->addItem(place.name);
        m_places->setEnabled(!places.isEmpty()); m_apply->setEnabled(!places.isEmpty());
        setStatus(places.isEmpty() ? tr("No locations found.") : tr("Select a location and click Apply."));
    });
    connect(m_weather, &WeatherService::searchFailed, this, [this](const QString &message) { m_search->setEnabled(true); setStatus(message); });
    connect(m_weather, &WeatherService::currentReady, this, [this](double temperature, int humidity, const QDateTime &observed) {
        m_weatherRequestPending=false;
        meteoReadings={temperature,humidity,m_locationName,QDateTime::currentDateTimeUtc(),true};
        m_temperature->setText(QLocale(QLocale::Spanish, QLocale::Spain).toString(temperature, 'f', 1)+" °C");
        m_humidity->setText(QString::number(humidity)+" %"); m_lastUpdate=QDateTime::currentDateTimeUtc();
        const QString info=tr("Open-Meteo · weather time: %1").arg(observed.toLocalTime().toString("dd/MM/yyyy HH:mm"));
        m_temperature->setToolTip(info); m_humidity->setToolTip(info); setStatus({});
    });
    connect(m_weather, &WeatherService::weatherFailed, this, [this](const QString &message) {
        m_weatherRequestPending=false;
        meteoReadings.available=false;
        m_temperature->setText("-- °C"); m_humidity->setText("-- %"); m_lastUpdate={}; setStatus(message);
    });
    m_clockTimer = new QTimer(this); m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &MeteoClock::updateClock);
    m_weatherTimer = new QTimer(this); m_weatherTimer->setInterval(10*60*1000);
    connect(m_weatherTimer, &QTimer::timeout, this, &MeteoClock::refreshWeather);
    loadSettings(); updateClock();
    m_options->setChecked(!m_hasLocation); m_settings->setVisible(!m_hasLocation);
    if (!m_hasLocation) setStatus(tr("Choose a city or use system location."));
}
MeteoClock::~MeteoClock()
{
    if (meteoClockInstance==this)
        meteoClockInstance.clear();
    m_weather->cancel();
    m_systemLocation->cancel();
}
MeteoClock::Readings MeteoClock::currentReadings() { return meteoReadings; }
bool MeteoClock::hasFreshReadings()
{
    if (!meteoReadings.available || !meteoReadings.updated.isValid())
        return false;
    const qint64 age=meteoReadings.updated.secsTo(QDateTime::currentDateTimeUtc());
    return age>=0 && age<=1800;
}
bool MeteoClock::requestCurrentReadings()
{
    if (!meteoClockInstance || !meteoClockInstance->m_hasLocation)
        return false;
    if (hasFreshReadings())
        return true;

    MeteoClock *clock=meteoClockInstance;
    if (!clock->m_weatherRequestPending
        && (!clock->m_lastWeatherRequest.isValid()
            || clock->m_lastWeatherRequest.secsTo(QDateTime::currentDateTimeUtc())>=60)) {
        clock->m_weatherRequestPending=true;
        clock->m_lastWeatherRequest=QDateTime::currentDateTimeUtc();
        clock->m_weather->requestCurrent(clock->m_latitude,clock->m_longitude);
    }
    return true;
}
QString MeteoClock::formatDate(const QDate &date)
{
    return QLocale(QLocale::Spanish, QLocale::Spain).toString(date, "dddd dd 'de' MMMM yyyy");
}
void MeteoClock::updateClock()
{
    const auto now=QDateTime::currentDateTime(); m_time->setText(now.toString("HH:mm:ss")); m_date->setText(formatDate(now.date()));
    updateClockFont();
}
void MeteoClock::updateClockFont()
{
    QFont font=m_time->font(); font.setPointSize(32);
    while (font.pointSize()>14 && QFontMetrics(font).horizontalAdvance("00:00:00")>m_time->width()-4) font.setPointSize(font.pointSize()-1);
    m_time->setFont(font);
}
void MeteoClock::setStatus(const QString &text) { m_status->setText(text); m_status->setVisible(!text.isEmpty()); }
void MeteoClock::setLocation(const QString &name, double latitude, double longitude)
{
    if (name.trimmed().isEmpty() || !validCoordinates(latitude,longitude)) return;
    m_weather->cancel(); m_weatherRequestPending=false; m_lastWeatherRequest={};
    m_locationName=name.trimmed(); m_latitude=latitude; m_longitude=longitude; m_hasLocation=true; m_lastUpdate={};
    meteoReadings.available=false;
    m_location->setText(m_locationName); m_temperature->setText("-- °C"); m_humidity->setText("-- %"); saveSettings();
    if (isVisible()) refreshWeather();
}
void MeteoClock::refreshWeather()
{
    if (!m_hasLocation || m_weatherRequestPending) return;
    m_weatherRequestPending=true;
    m_lastWeatherRequest=QDateTime::currentDateTimeUtc();
    setStatus(tr("Updating weather...")); m_weather->requestCurrent(m_latitude,m_longitude);
}
void MeteoClock::searchCity()
{
    m_systemLocation->cancel(); m_autoLocation->setEnabled(true);
    m_results.clear(); m_places->clear(); m_places->setEnabled(false); m_apply->setEnabled(false);
    m_search->setEnabled(false); setStatus(tr("Searching city...")); m_weather->searchCity(m_city->text());
}
void MeteoClock::loadSettings()
{
    QSettings settings(settingsPath(),QSettings::IniFormat);
    m_locationName=settings.value("name").toString(); bool latOk=false,lonOk=false;
    m_latitude=settings.value("latitude").toDouble(&latOk); m_longitude=settings.value("longitude").toDouble(&lonOk);
    m_hasLocation=latOk && lonOk && !m_locationName.isEmpty() && validCoordinates(m_latitude,m_longitude);
    if (m_hasLocation) m_location->setText(m_locationName);
}
void MeteoClock::saveSettings()
{
    QSettings settings(settingsPath(),QSettings::IniFormat);
    settings.setValue("name",m_locationName); settings.setValue("latitude",m_latitude); settings.setValue("longitude",m_longitude);
}
void MeteoClock::showEvent(QShowEvent *event)
{
    Frame::showEvent(event); updateClock(); m_clockTimer->start(); m_weatherTimer->start();
    if (m_hasLocation && (!m_lastUpdate.isValid() || m_lastUpdate.secsTo(QDateTime::currentDateTimeUtc())>=600)) refreshWeather();
}
void MeteoClock::hideEvent(QHideEvent *event)
{
    Frame::hideEvent(event); m_clockTimer->stop(); m_weather->cancelSearch(); m_systemLocation->cancel();
    m_autoLocation->setEnabled(true); m_search->setEnabled(true);
}
void MeteoClock::resizeEvent(QResizeEvent *event) { Frame::resizeEvent(event); updateClockFont(); }
