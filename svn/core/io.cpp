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
#include <QWidget>


#include "core/io.h"
#include "widgets/AudioItemFile.h"

Io::Io(QObject *parent): QObject(parent){

}


Io::~Io(){}



void Io::saveWidgetsToJson(QLayout* layout, const QString& filename){
    QJsonObject rootObj;  // Objeto raíz del JSON

       //  Metadatos
       QJsonObject metadataObj;
       metadataObj["description"] = "Rdit list";
       metadataObj["totalitem"] = layout->count();
       rootObj["metadata"] = metadataObj;  // Se guarda en un campo "metadata"

       //  Lista de widgets
       QJsonArray widgetsArray;
       for (int i = 0; i < layout->count(); ++i) {
           QLayoutItem* item = layout->itemAt(i);

           if (item->widget()) {
               QWidget* widget = item->widget();
               QJsonObject widgetObj;
               widgetObj["type"] = widget->metaObject()->className();
               widgetObj["name"] = widget->objectName();

                  if (AudioItemFile* audioitemfile = qobject_cast<AudioItemFile*>(widget)){
                        widgetObj["nombre"] = audioitemfile->nombre->text();
                        widgetObj["filepath"] = audioitemfile->getfilePath();
                  }

               widgetsArray.append(widgetObj);
           }
       }
       rootObj["audioitems"] = widgetsArray;  // Se guarda en un campo "audioitem"

       // Guardar el JSON
       QJsonDocument doc(rootObj);  // Usamos rootObj en lugar de widgetsArray
       QFile file(filename);
       if (file.open(QIODevice::WriteOnly)) {
           file.write(doc.toJson());
           file.close();
       }
}



//***************************************************


void Io::loadWidgetsFromJson(QLayout* layout, const QString& filename) {
    // Limpiar el layout actual (opcional, depende de tu caso de uso)
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Leer el archivo JSON
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "No se pudo abrir el archivo:" << filename;
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // Parsear el JSON
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qWarning() << "JSON inválido:" << filename;
        return;
    }

    QJsonObject rootObj = doc.object();

    // Verificar si existe la sección de audioitems
    if (!rootObj.contains("audioitems") || !rootObj["audioitems"].isArray()) {
        qWarning() << "Formato JSON incorrecto: falta 'audioitems'";
        return;
    }

    QJsonArray widgetsArray = rootObj["audioitems"].toArray();

    // Crear los widgets y añadirlos al layout
    for (const QJsonValue& value : widgetsArray) {
            QJsonObject widgetObj = value.toObject();

            QString type = widgetObj["type"].toString();
            QString name = widgetObj["name"].toString();

        if (type == "AudioItemFile") {
            // Crear el widget personalizado AudioItemFile
            AudioItemFile* audioItem = new AudioItemFile();
            audioItem->setObjectName(name);

            // Establecer propiedades específicas
            if (widgetObj.contains("nombre")) {
                audioItem->nombre->setText(widgetObj["nombre"].toString());
            }
            if (widgetObj.contains("filepath")) {
                audioItem->setfilePath(widgetObj["filepath"].toString());
            }

            // Añadir al layout
            layout->addWidget(audioItem);
        }
    }
}


//***********===============guardado para estudio =============

//=================================================================


/*void Io::saveWidgetsToJson(QLayout* layout, const QString& filename) {
    QJsonObject rootObj;

    // Metadatos
    QJsonObject metadataObj;
    metadataObj["description"] = "Rdit list";
    metadataObj["totalitem"] = layout->count();
    rootObj["metadata"] = metadataObj;

    // Lista de widgets
    QJsonArray widgetsArray;
    for (int i = 0; i < layout->count(); ++i) {
        QLayoutItem* item = layout->itemAt(i);
        if (item->widget()) {
            QWidget* widget = item->widget();
            QJsonObject widgetObj;
            widgetObj["type"] = widget->metaObject()->className();
            widgetObj["name"] = widget->objectName();

            if (AudioItemFile* audioitemfile = qobject_cast<AudioItemFile*>(widget)) {
                // Guardar AudioItemFile
                widgetObj["nombre"] = audioitemfile->nombre->text();
                widgetObj["filepath"] = audioitemfile->getfilePath();
            }
            else if (AudioItemGroup* audioitemgroup = qobject_cast<AudioItemGroup*>(widget)) {
                // Guardar AudioItemGroup (recursivamente)
                QJsonArray groupItemsArray;
                QLayout* groupLayout = audioitemgroup->getLayout(); // Asume que existe este método

                for (int j = 0; j < groupLayout->count(); ++j) {
                    QLayoutItem* groupItem = groupLayout->itemAt(j);
                    if (groupItem->widget()) {
                        QWidget* groupWidget = groupItem->widget();
                        QJsonObject groupWidgetObj;
                        groupWidgetObj["type"] = groupWidget->metaObject()->className();
                        groupWidgetObj["name"] = groupWidget->objectName();

                        if (AudioItemFile* groupAudioItem = qobject_cast<AudioItemFile*>(groupWidget)) {
                            groupWidgetObj["nombre"] = groupAudioItem->nombre->text();
                            groupWidgetObj["filepath"] = groupAudioItem->getfilePath();
                        }

                        groupItemsArray.append(groupWidgetObj);
                    }
                }
                widgetObj["group_items"] = groupItemsArray;
                widgetObj["group_name"] = audioitemgroup->getGroupName(); // Asume que existe
            }

            widgetsArray.append(widgetObj);
        }
    }

    rootObj["audioitems"] = widgetsArray;

    // Guardar JSON
    QJsonDocument doc(rootObj);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}*/










