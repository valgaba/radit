/* This file is part of Radit.
   Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   radit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "core/config.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

#include "widgets/Player.h"


// ============================================================
// Ruta del fichero de configuración
// ============================================================

static QString configFilePath()
{
    return QDir(QCoreApplication::applicationDirPath())
        .filePath("config.json");
}


// ============================================================
// SAVE
// ============================================================

bool Config::saveConfig(const QString& filename,
                        const QList<Player*>& players)
{
    Q_UNUSED(filename);

    QJsonObject root;

    root["version"] = 1;

    QJsonArray playersArray;

    for (int i = 0; i < players.size(); ++i) {

        Player* player = players.at(i);

        if (!player)
            continue;

        QJsonObject playerObject;

        // ----------------------------------------
        // Identificador del Player
        // ----------------------------------------

        playerObject["id"] = player->objectName();

        if (playerObject["id"].toString().isEmpty()) {
            playerObject["id"] =
                QString("Player%1").arg(i + 1);
        }


        // ----------------------------------------
        // PLAY DEVICE
        // ----------------------------------------

        QJsonObject playDevice;

        int playIndex = player->devicePlay();

        playDevice["index"] = playIndex;


        // ----------------------------------------
        // CUE DEVICE
        // ----------------------------------------

        QJsonObject cueDevice;

        int cueIndex = player->deviceCue();

        cueDevice["index"] = cueIndex;


        playerObject["playDevice"] = playDevice;
        playerObject["cueDevice"] = cueDevice;

        playersArray.append(playerObject);
    }

    root["players"] = playersArray;


    // ----------------------------------------
    // Crear documento JSON
    // ----------------------------------------

    QJsonDocument document(root);


    // ----------------------------------------
    // Guardar junto al EXE
    // ----------------------------------------

    QString configPath = configFilePath();

    QFile file(configPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {

        qWarning() << "No se puede guardar configuración:"
                   << configPath;

        return false;
    }

    file.write(document.toJson(QJsonDocument::Indented));

    file.close();
    return true;
}


// ============================================================
// LOAD
// ============================================================

bool Config::loadConfig(const QString& filename,
                        const QList<Player*>& players)
{
    Q_UNUSED(filename);


    // ----------------------------------------
    // Leer junto al EXE
    // ----------------------------------------

    QString configPath = configFilePath();

    QFile file(configPath);

    if (!file.open(QIODevice::ReadOnly)) {

        qWarning() << "No se puede abrir configuración:"
                   << configPath;

        return false;
    }


    QByteArray data = file.readAll();

    file.close();


    // ----------------------------------------
    // Parsear JSON
    // ----------------------------------------

    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(data, &error);


    if (error.error != QJsonParseError::NoError) {

        qWarning() << "Error leyendo configuración:"
                   << error.errorString();

        return false;
    }


    if (!document.isObject()) {

        qWarning() << "Configuración inválida:"
                   << "el elemento raíz no es un objeto";

        return false;
    }


    QJsonObject root = document.object();


    // ----------------------------------------
    // Comprobar players
    // ----------------------------------------

    if (!root.contains("players") ||
        !root["players"].isArray()) {

        qWarning() << "Configuración inválida:"
                   << "no existe 'players'";

        return false;
    }


    QJsonArray playersArray =
        root["players"].toArray();


    // ----------------------------------------
    // Recorrer Players guardados
    // ----------------------------------------

    for (const QJsonValue& value : playersArray) {

        if (!value.isObject())
            continue;


        QJsonObject playerObject =
            value.toObject();


        QString playerId =
            playerObject["id"].toString();


        // ------------------------------------
        // Buscar Player correspondiente
        // ------------------------------------

        Player* playerEncontrado = nullptr;


        for (Player* player : players) {

            if (!player)
                continue;


            if (player->objectName() == playerId) {

                playerEncontrado = player;

                break;
            }
        }


        if (!playerEncontrado) {

            qWarning() << "Player no encontrado:"
                       << playerId;

            continue;
        }


        // ------------------------------------
        // PLAY DEVICE
        // ------------------------------------

        int playDevice = -1;


        if (playerObject.contains("playDevice") &&
            playerObject["playDevice"].isObject()) {

            QJsonObject playObject =
                playerObject["playDevice"].toObject();


            playDevice =
                playObject["index"].toInt(-1);
        }


        // ------------------------------------
        // CUE DEVICE
        // ------------------------------------

        int cueDevice = -1;


        if (playerObject.contains("cueDevice") &&
            playerObject["cueDevice"].isObject()) {

            QJsonObject cueObject =
                playerObject["cueDevice"].toObject();


            cueDevice =
                cueObject["index"].toInt(-1);
        }


        // ------------------------------------
        // Aplicar configuración
        // ------------------------------------

        if (playDevice >= 0)
            playerEncontrado->setDevicePlay(playDevice);


        if (cueDevice >= 0)
            playerEncontrado->setDeviceCue(cueDevice);


    }


    return true;
}
