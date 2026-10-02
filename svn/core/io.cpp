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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

#include "core/io.h"
#include "widgets/AudioItemMaxi.h"
#include "widgets/contentsbase.h"
#include "widgets/AudioItemFilemaxi.h"

Io::Io(QObject *parent): QObject(parent){

}


Io::~Io(){}

void Io::saveContentsPlayer(QLayout* layout, const QString& filename)
{
    QJsonObject rootObj;

    rootObj["version"] = 1;

    QJsonArray itemsArray;

    for (int i = 0; i < layout->count(); ++i) {

        QLayoutItem* layoutItem = layout->itemAt(i);

        if (!layoutItem)
            continue;

        QWidget* widget = layoutItem->widget();

        if (!widget)
            continue;

        AudioItemMaxi* item =
            qobject_cast<AudioItemMaxi*>(widget);

        if (!item)
            continue;

        QJsonObject itemObj;

        itemObj["url"] = item->filePath();
        itemObj["name"] = item->nameFile();
        itemObj["second"] = item->second();

        itemObj["select"] = item->isSelect();
        itemObj["playNext"] = item->isPlayNext();
        itemObj["purge"] = item->isPurge();
        itemObj["loop"] = item->isLoop();

        // Color
        itemObj["color"] = item->color().name(QColor::HexArgb);

        itemsArray.append(itemObj);
    }

    rootObj["count"] = itemsArray.count();
    rootObj["items"] = itemsArray;

    QJsonDocument document(rootObj);

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "No se pudo abrir el fichero para escribir:"
                   << filename;
        return;
    }

    file.write(document.toJson(QJsonDocument::Indented));
    file.close();

    qDebug() << "Lista guardada en:" << filename;
}




void Io::loadContentsPlayer(ContentsBase* contents, const QString& filename)
{
    if (!contents)
        return;

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir el fichero para leer:"
                   << filename;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Error leyendo JSON:"
                   << error.errorString();
        return;
    }

    if (!document.isObject()) {
        qWarning() << "El fichero no contiene un objeto JSON";
        return;
    }

    QJsonObject rootObj = document.object();

    int version = rootObj["version"].toInt(1);

    if (version != 1) {
        qWarning() << "Versión de lista no soportada:"
                   << version;
        return;
    }

    QJsonArray itemsArray = rootObj["items"].toArray();

    int count = rootObj["count"].toInt(itemsArray.count());


    for (const QJsonValue &value : itemsArray) {

        if (!value.isObject())
            continue;

        QJsonObject itemObj = value.toObject();

        QString filePath = itemObj["url"].toString();

        if (filePath.isEmpty()) {
            qWarning() << "Item sin URL, se omite";
            continue;
        }

        // Crear el item
        AudioItemFileMaxi* item = new AudioItemFileMaxi(contents);

        // Datos principales
        item->setFilePath(filePath);
        item->setNameFile(itemObj["name"].toString());

        double second = itemObj["second"].toDouble();

        item->setSecond(second);
        item->setTiempoFile(second);

        // Estados
        item->setIsSelect(
            itemObj["select"].toBool(false)
        );

        item->setIsPlayNext(
            itemObj["playNext"].toBool(false)
        );

        item->setIsPurge(
            itemObj["purge"].toBool(false)
        );

        item->setIsLoop(
            itemObj["loop"].toBool(false)
        );

        // Color
        if (itemObj.contains("color")) {

            QColor color(itemObj["color"].toString());

            if (color.isValid()) {
                item->setColor(color);
            }
        }

        // Añadir y conectar exactamente igual
        // que cualquier item creado normalmente.
        contents->createItem(item);
    }

    qDebug() << "Lista cargada correctamente:" << filename;
}



