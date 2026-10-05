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
#include <QFileInfo>
#include <QSaveFile>
#include <QSignalBlocker>
#include <cmath>
#include "core/io.h"
#include "widgets/AudioItemMaxi.h"
#include "widgets/contentsbase.h"
#include "widgets/AudioItemFilemaxi.h"
#include "widgets/TabPlayer.h"
#include "widgets/tabbar.h"
#include "widgets/container.h"

namespace {
bool fail(QString *error, const QString &message)
{
    if (error) *error = message;
    return false;
}

QJsonArray saveItems(QLayout *layout)
{
    QJsonArray items;
    if (!layout) return items;
    for (int i = 0; i < layout->count(); ++i) {
        auto *item = qobject_cast<AudioItemMaxi*>(layout->itemAt(i)->widget());
        if (!item) continue;
        items.append(QJsonObject{
            {"url", item->filePath()}, {"name", item->nameFile()},
            {"second", item->second()}, {"secondStart", item->secondStart()},
            {"select", item->isSelect()}, {"playNext", item->isPlayNext()},
            {"purge", item->isPurge()}, {"loop", item->isLoop()},
            {"color", item->color().name(QColor::HexArgb)}
        });
    }
    return items;
}

void loadItems(ContentsBase *contents, const QJsonArray &items)
{
    for (const auto &value : items) {
        if (!value.isObject()) continue;
        const auto object = value.toObject();
        const QString path = object["url"].toString();
        if (path.isEmpty()) continue;
        auto *item = new AudioItemFileMaxi(contents);
        item->setFilePath(path);
        item->setToolTip(path);
        item->setNameFile(object["name"].toString());
        const double seconds = object["second"].toDouble();
        item->setSecond(seconds);
        item->setTiempoFile(seconds);
        item->setSecondStart(object["secondStart"].toDouble());
        item->setIsSelect(object["select"].toBool());
        item->setIsPlayNext(object["playNext"].toBool());
        item->setIsPurge(object["purge"].toBool());
        item->setIsLoop(object["loop"].toBool());
        const QColor color(object["color"].toString());
        if (color.isValid()) item->setColor(color);
        contents->createItem(item);
    }
}

bool writeJson(const QString &filename, const QJsonObject &root, QString *error)
{
    QSaveFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return fail(error, Io::tr("Cannot save %1: %2").arg(filename, file.errorString()));
    const QByteArray bytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(bytes) != bytes.size() || !file.commit())
        return fail(error, Io::tr("Cannot save %1: %2").arg(filename, file.errorString()));
    return true;
}

bool readJson(const QString &filename, QJsonObject &root, QString *error)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return fail(error, Io::tr("Cannot open %1: %2").arg(filename, file.errorString()));
    const QByteArray bytes = file.readAll();
    if (file.error() != QFileDevice::NoError)
        return fail(error, file.errorString());
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail(error, Io::tr("Invalid JSON: %1").arg(parseError.errorString()));
    if (!document.isObject())
        return fail(error, Io::tr("The file must contain a JSON object."));
    root = document.object();
    return true;
}

bool validItems(const QJsonArray &items)
{
    for (const auto &value : items) {
        if (!value.isObject()) return false;
        const auto item = value.toObject();
        if (!item["url"].isString() || item["url"].toString().isEmpty()
            || !item["name"].isString() || !item["second"].isDouble()
            || item["second"].toDouble() < 0 || !std::isfinite(item["second"].toDouble()))
            return false;
        for (const auto *key : {"select", "playNext", "purge", "loop"})
            if (item.contains(key) && !item[key].isBool()) return false;
        if (item.contains("color") && !QColor(item["color"].toString()).isValid()) return false;
        if (item.contains("secondStart") && (!item["secondStart"].isDouble()
            || item["secondStart"].toDouble() < 0
            || !std::isfinite(item["secondStart"].toDouble()))) return false;
    }
    return true;
}
}

Io::Io(QObject *parent) : QObject(parent) {}
Io::~Io() {}

bool Io::SaveListPlayer(QLayout *layout, const QString &filename, QString *error)
{
    if (error) error->clear();
    if (!layout) return fail(error, tr("No list was selected."));
    const QJsonArray items = saveItems(layout);
    return writeJson(filename, QJsonObject{{"version", 1}, {"count", items.size()}, {"items", items}}, error);
}

bool Io::LoadListPlayer(ContentsBase *contents, const QString &filename, QString *error)
{
    if (error) error->clear();
    if (!contents) return fail(error, tr("No list was selected."));
    QJsonObject root;
    if (!readJson(filename, root, error)) return false;
    if (root["version"].toDouble(1) != 1 || !root["items"].isArray())
        return fail(error, tr("Unsupported list format or version."));
    if (!validItems(root["items"].toArray()))
        return fail(error, tr("Invalid data in the list."));
    loadItems(contents, root["items"].toArray());
    if (auto *list = qobject_cast<ContentsPlayer*>(contents))
        list->setListFileName(QFileInfo(filename).absoluteFilePath());
    return true;
}

bool Io::SavePlayer(TabPlayer *player, const QString &filename, QString *error)
{
    if (error) error->clear();
    if (!player || player->count() == 0)
        return fail(error, tr("The player has no tabs to save."));
    auto *bar = player->findChild<TabBar*>();
    QJsonArray tabs;
    for (int index = 0; index < player->count(); ++index) {
        auto *container = qobject_cast<Container*>(player->widget(index));
        auto *contents = container ? qobject_cast<ContentsPlayer*>(container->widget()) : nullptr;
        if (!contents || !bar)
            return fail(error, tr("Cannot read the contents of tab %1.").arg(index + 1));
        const QColor color = bar->tabColor(index);
        tabs.append(QJsonObject{{"name", player->tabText(index)},
            {"listFile", contents->listFileName()},
            {"color", color.isValid() ? color.name(QColor::HexArgb) : QColor(Qt::transparent).name(QColor::HexArgb)},
            {"items", saveItems(contents->layout)}});
    }
    const QJsonObject root{{"format", "radit-player"}, {"version", 1},
        {"currentTab", player->currentIndex()}, {"tabs", tabs}};
    if (!writeJson(filename, root, error)) return false;
    player->setPlayerFileName(QFileInfo(filename).absoluteFilePath());
    return true;
}

bool Io::LoadPlayer(TabPlayer *player, const QString &filename, QString *error)
{
    if (error) error->clear();
    if (!player) return fail(error, tr("No player was selected."));
    auto *bar = player->findChild<TabBar*>();
    if (!bar) return fail(error, tr("The player has no tab bar."));
    QJsonObject root;
    if (!readJson(filename, root, error)) return false;
    if (root["format"].toString() != "radit-player" || root["version"].toDouble() != 1
        || !root["tabs"].isArray() || root["tabs"].toArray().isEmpty())
        return fail(error, tr("Unsupported player format or version."));
    const QJsonArray tabs = root["tabs"].toArray();
    const auto active = root["currentTab"];
    const int currentIndex = active.toInt(-1);
    if (!active.isDouble() || active.toDouble() != currentIndex
        || currentIndex < 0 || currentIndex >= tabs.size())
        return fail(error, tr("Invalid selected tab."));
    for (const auto &value : tabs) {
        if (!value.isObject()) return fail(error, tr("Invalid tab data."));
        const auto tab = value.toObject();
        if (!tab["name"].isString() || !tab["items"].isArray()
            || (tab.contains("listFile") && !tab["listFile"].isString())
            || !validItems(tab["items"].toArray())
            || (tab.contains("color") && !QColor(tab["color"].toString()).isValid()))
            return fail(error, tr("Invalid data in a player tab."));
    }

    // The full document is validated before replacing any existing tabs.
    QWidget *owner = player->parentWidget();
    while (owner && !qobject_cast<Player*>(owner)) owner = owner->parentWidget();
    if (auto *audioPlayer = qobject_cast<Player*>(owner)) audioPlayer->stopMain();
    QSignalBlocker blocker(player);
    while (player->count() > 0) {
        QWidget *page = player->widget(0);
        auto &clipboard = Clipboard::instance().lista;
        for (auto it = clipboard.begin(); it != clipboard.end();) {
            if (*it == page || page->isAncestorOf(*it)) it = clipboard.erase(it);
            else ++it;
        }
        player->removeTab(0);
        delete page;
    }
    for (const auto &value : tabs) {
        const auto tab = value.toObject();
        auto *container = new Container(player);
        const QString name = tab["name"].toString();
        const int index = player->addTab(container, name);
        player->setTabToolTip(index, name);
        bar->setTabColor(index, QColor(tab["color"].toString()));
        auto *contents = qobject_cast<ContentsPlayer*>(container->widget());
        loadItems(contents, tab["items"].toArray());
        contents->setListFileName(tab["listFile"].toString());
    }
    player->setCurrentIndex(currentIndex);
    blocker.unblock();
    player->setPlayerFileName(QFileInfo(filename).absoluteFilePath());
    return true;
}

