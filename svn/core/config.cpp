#include "core/config.h"
#include "core/MediaManager.h"
#include "widgets/Player.h"
#include "widgets/Capture.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <cmath>

namespace {
QString configPath(const QString &filename)
{
    return QDir::isAbsolutePath(filename) ? filename
        : QDir(QCoreApplication::applicationDirPath()).filePath(filename);
}

QJsonObject deviceSettings(int id, const QList<AudioDevice> &devices)
{
    QJsonObject result;
    for (const auto &device : devices) {
        if (device.id == id) {
            result["name"] = device.name;
            result["key"] = device.key;
            break;
        }
    }
    return result;
}

int resolveDevice(const QJsonObject &saved, const QList<AudioDevice> &devices)
{
    const QString key = saved.value("key").toString();
    const QString name = saved.value("name").toString();
    // Never interpret an old index as another device when an identity was saved.
    if (!key.isEmpty()) {
        for (const auto &device : devices)
            if (device.key == key) return device.id;
    } else if (!name.isEmpty()) {
        int match = -1;
        for (const auto &device : devices) {
            if (device.name != name) continue;
            if (match >= 0) { match = -1; break; }
            match = device.id;
        }
        if (match >= 0) return match;
    }
    for (const auto &device : devices)
        if (device.isDefault) return device.id;
    return devices.isEmpty() ? -1 : devices.first().id;
}

float savedVolume(const QJsonObject &object)
{
    const double value = object.value("volume").toDouble(1.0);
    return std::isfinite(value) ? static_cast<float>(qBound(0.0, value, 1.0)) : 1.0f;
}
}

bool Config::saveConfig(const QString &filename, const QList<Player *> &players,
                        Capture *capture, QString *error)
{
    if (error) error->clear();
    const auto outputs = MediaManager::outputDevices();
    QJsonArray playerArray;
    for (int i = 0; i < players.size(); ++i) {
        const Player *player = players.at(i);
        if (!player) continue;
        playerArray.append(QJsonObject{
            {"id", player->objectName().isEmpty() ? QString("Player%1").arg(i + 1) : player->objectName()},
            {"playDevice", deviceSettings(player->devicePlay(), outputs)},
            {"cueDevice", deviceSettings(player->deviceCue(), outputs)},
            {"volume", player->committedVolume()}
        });
    }
    QJsonObject root{{"version", 2}, {"players", playerArray}};
    if (capture) {
        root["capture"] = QJsonObject{
            {"inputDevice", deviceSettings(capture->inputDevice(), MediaManager::inputDevices())},
            {"volume", capture->inputVolume()},
            {"recordingMode", capture->recordingMode()}
        };
    }
    QSaveFile file(configPath(filename));
    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (!file.open(QIODevice::WriteOnly) || file.write(json) != json.size() || !file.commit()) {
        if (error) *error = QCoreApplication::translate("Config", "Could not save %1: %2")
            .arg(file.fileName(), file.errorString());
        return false;
    }
    return true;
}

bool Config::loadConfig(const QString &filename, const QList<Player *> &players,
                        Capture *capture, QString *error)
{
    if (error) error->clear();
    QFile file(configPath(filename));
    if (!file.exists()) return true; // First start uses the normal defaults.
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = file.errorString();
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()
        || document.object().value("version").toInt() != 2
        || !document.object().value("players").isArray()) {
        if (error) *error = QCoreApplication::translate("Config", "Invalid configuration in %1")
            .arg(file.fileName());
        return false;
    }
    const QJsonObject root = document.object();
    const auto outputs = MediaManager::outputDevices();
    for (const auto &value : root.value("players").toArray()) {
        const QJsonObject saved = value.toObject();
        for (Player *player : players) {
            if (!player || player->objectName() != saved.value("id").toString()) continue;
            if (saved.value("playDevice").isObject())
                player->setDevicePlay(resolveDevice(saved.value("playDevice").toObject(), outputs));
            if (saved.value("cueDevice").isObject())
                player->setDeviceCue(resolveDevice(saved.value("cueDevice").toObject(), outputs));
            if (!player->setVolume(savedVolume(saved))) {
                if (error) *error = QCoreApplication::translate("Config", "Could not restore playback volume.");
                return false;
            }
        }
    }
    if (capture && root.value("capture").isObject()) {
        const QJsonObject saved = root.value("capture").toObject();
        if (saved.value("inputDevice").isObject())
            capture->setInputDevice(resolveDevice(saved.value("inputDevice").toObject(), MediaManager::inputDevices()));
        capture->setInputVolume(savedVolume(saved));
        if (!capture->setRecordingMode(saved.value("recordingMode").toString("mp3-44100-stereo-192"))) {
            if (error) *error = QCoreApplication::translate("Config", "Invalid MP3 recording mode.");
            return false;
        }
    }
    return true;
}
