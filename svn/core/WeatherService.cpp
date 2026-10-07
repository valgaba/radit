/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "core/WeatherService.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <cmath>

namespace {
bool coordinatesValid(double latitude, double longitude)
{
    return std::isfinite(latitude) && std::isfinite(longitude)
        && latitude >= -90 && latitude <= 90 && longitude >= -180 && longitude <= 180;
}
QNetworkRequest request(const QUrl &url)
{
    QNetworkRequest result(url);
    result.setHeader(QNetworkRequest::UserAgentHeader, "Radit MeteoClock/1.0");
    result.setTransferTimeout(15000);
    return result;
}
void abort(QPointer<QNetworkReply> &current)
{
    const auto reply = current; current.clear();
    if (reply) reply->abort();
}
}
WeatherService::WeatherService(QObject *parent, QNetworkAccessManager *network)
    : QObject(parent), m_network(network ? network : new QNetworkAccessManager(this)) {}
WeatherService::~WeatherService() { cancel(); }
void WeatherService::cancel() { abort(m_searchReply); abort(m_weatherReply); }
void WeatherService::cancelSearch() { abort(m_searchReply); }

void WeatherService::searchCity(const QString &name)
{
    abort(m_searchReply);
    if (name.trimmed().size() < 2) { emit searchFailed(tr("Enter a city name.")); return; }
    QUrl url("https://geocoding-api.open-meteo.com/v1/search");
    QUrlQuery query;
    query.addQueryItem("name", name.trimmed()); query.addQueryItem("count", "10");
    query.addQueryItem("language", "es"); query.addQueryItem("format", "json"); url.setQuery(query);
    auto *reply = m_network->get(request(url)); m_searchReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_searchReply != reply) return;
        m_searchReply.clear();
        if (reply->error() != QNetworkReply::NoError) { emit searchFailed(tr("City search is unavailable. Please try again.")); return; }
        QJsonParseError error;
        const auto document = QJsonDocument::fromJson(reply->readAll(), &error);
        if (error.error != QJsonParseError::NoError || !document.isObject()) { emit searchFailed(tr("Invalid location response.")); return; }
        QList<WeatherPlace> places;
        for (const auto &value : document.object()["results"].toArray()) {
            const auto object = value.toObject();
            if (!object["latitude"].isDouble() || !object["longitude"].isDouble() || object["name"].toString().isEmpty()) continue;
            WeatherPlace place; place.latitude = object["latitude"].toDouble(); place.longitude = object["longitude"].toDouble();
            if (!coordinatesValid(place.latitude, place.longitude)) continue;
            QStringList names{object["name"].toString()};
            for (const auto *key : {"admin1", "country"}) {
                const QString text = object[key].toString(); if (!text.isEmpty() && !names.contains(text)) names.append(text);
            }
            place.name = names.join(", "); places.append(place);
        }
        emit placesReady(places);
    });
}

void WeatherService::requestCurrent(double latitude, double longitude)
{
    abort(m_weatherReply);
    if (!coordinatesValid(latitude, longitude)) { emit weatherFailed(tr("Invalid location.")); return; }
    QUrl url("https://api.open-meteo.com/v1/forecast"); QUrlQuery query;
    query.addQueryItem("latitude", QString::number(latitude, 'f', 6));
    query.addQueryItem("longitude", QString::number(longitude, 'f', 6));
    query.addQueryItem("current", "temperature_2m,relative_humidity_2m");
    query.addQueryItem("temperature_unit", "celsius"); query.addQueryItem("timeformat", "unixtime");
    query.addQueryItem("timezone", "auto"); query.addQueryItem("forecast_days", "1"); url.setQuery(query);
    auto *reply = m_network->get(request(url)); m_weatherReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (m_weatherReply != reply) return;
        m_weatherReply.clear();
        if (reply->error() != QNetworkReply::NoError) { emit weatherFailed(tr("Weather unavailable. Please try again.")); return; }
        QJsonParseError error; const auto document = QJsonDocument::fromJson(reply->readAll(), &error);
        const auto current = document.object()["current"].toObject();
        if (error.error != QJsonParseError::NoError || !document.isObject()
            || !current["temperature_2m"].isDouble() || !current["relative_humidity_2m"].isDouble() || !current["time"].isDouble()) {
            emit weatherFailed(tr("Invalid weather response.")); return;
        }
        const double temperature = current["temperature_2m"].toDouble(), humidity = current["relative_humidity_2m"].toDouble();
        const double timestamp = current["time"].toDouble();
        if (!std::isfinite(temperature) || !std::isfinite(humidity) || humidity < 0 || humidity > 100
            || !std::isfinite(timestamp) || timestamp < 0 || timestamp > 4102444800.0) {
            emit weatherFailed(tr("Invalid weather response.")); return;
        }
        emit currentReady(temperature, qRound(humidity), QDateTime::fromSecsSinceEpoch(qint64(timestamp), Qt::UTC));
    });
}
