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
#include <QScopedValueRollback>
#include <QFutureWatcher>
#include <QEventLoop>
#include <QPointer>
#include <QtConcurrent/QtConcurrentRun>
#include "widgets/LoadingDialog.h"
#include <cmath>
#include "core/io.h"
#include "widgets/AudioItemMaxi.h"
#include "widgets/AudioItemNetMaxi.h"
#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/contentsbase.h"
#include "widgets/AudioItemFilemaxi.h"
#include "widgets/TabPlayer.h"
#include "widgets/tabbar.h"
#include "widgets/container.h"

namespace {
bool importInProgress = false;

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
            {"type", qobject_cast<AudioItemFolderMaxi*>(item) ? "folder" :
                         item->isLiveStream() ? "net" : "file"},
            {"url", item->filePath()}, {"name", item->nameFile()},
            {"second", item->second()}, {"secondStart", item->secondStart()},
            {"select", item->isSelect()}, {"playNext", item->isPlayNext()},
            {"purge", item->isPurge()}, {"loop", item->isLoop()},
            {"color", item->color().name(QColor::HexArgb)}
        });
    }
    return items;
}

void loadItems(ContentsBase *contents, const QJsonArray &items,
               LoadingDialog &loading, int &completed, int total)
{
    for (const auto &value : items) {
        if (!value.isObject()) continue;
        const auto object = value.toObject();
        const QString path = object["url"].toString();
        if (path.isEmpty()) continue;
        const bool network = MediaManager::isNetworkUrl(path);
        const bool folder = object["type"].toString() == "folder";
        AudioItemMaxi *item = folder ? static_cast<AudioItemMaxi*>(new AudioItemFolderMaxi(contents)) :
            network ? static_cast<AudioItemMaxi*>(new AudioItemNetMaxi(contents)) :
                      static_cast<AudioItemMaxi*>(new AudioItemFileMaxi(contents));
        if (folder) static_cast<AudioItemFolderMaxi*>(item)->setFolderPath(path);
        item->setFilePath(path);
        item->setToolTip(path);
        item->setNameFile(object["name"].toString());
        const double seconds = object["second"].toDouble();
        item->setSecond(seconds);
        if (!network && !folder) item->setTiempoFile(seconds);
        if (!folder) item->setSecondStart(object["secondStart"].toDouble());
        item->setIsSelect(object["select"].toBool());
        item->setIsPlayNext(object["playNext"].toBool());
        item->setIsPurge(object["purge"].toBool());
        if (!network) item->setIsLoop(object["loop"].toBool());
        const QColor color(object["color"].toString());
        if (color.isValid()) item->setColor(color);
        contents->createItem(item);
        loading.setProgress(++completed, total);
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

struct JsonReadResult {
    QJsonObject root;
    QString error;
};

bool readJson(const QString &filename, QJsonObject &root, QString *error)
{
    // Only file reading and JSON parsing run on the worker; no widgets are touched.
    QFutureWatcher<JsonReadResult> watcher;
    QEventLoop loop;
    QObject::connect(&watcher, &QFutureWatcher<JsonReadResult>::finished,
                     &loop, &QEventLoop::quit);
    watcher.setFuture(QtConcurrent::run([filename]() {
        JsonReadResult result;
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            result.error = Io::tr("Cannot open %1: %2").arg(filename, file.errorString());
            return result;
        }
        const QByteArray bytes = file.readAll();
        if (file.error() != QFileDevice::NoError) {
            result.error = file.errorString();
            return result;
        }
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(bytes, &parseError);
        if (parseError.error != QJsonParseError::NoError)
            result.error = Io::tr("Invalid JSON: %1").arg(parseError.errorString());
        else if (!document.isObject())
            result.error = Io::tr("The file must contain a JSON object.");
        else result.root = document.object();
        return result;
    }));
    if (!watcher.isFinished()) loop.exec(QEventLoop::ExcludeUserInputEvents);
    const auto result = watcher.result();
    if (!result.error.isEmpty()) return fail(error, result.error);
    root = result.root;
    return true;
}

bool validItems(const QJsonArray &items, LoadingDialog &loading)
{
    for (const auto &value : items) {
        loading.refresh();
        if (!value.isObject()) return false;
        const auto item = value.toObject();
        if (item.contains("type") && (!item["type"].isString() ||
            !QStringList{"file", "net", "folder"}.contains(item["type"].toString()))) return false;
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
    if (importInProgress) return fail(error, tr("Another import is already in progress."));
    QScopedValueRollback<bool> importing(importInProgress, true);
    QPointer<ContentsBase> target(contents);
    LoadingDialog loading(contents->window(), tr("Loading list…"));
    QJsonObject root;
    if (!readJson(filename, root, error)) return false;
    if (root["version"].toDouble(1) != 1 || !root["items"].isArray())
        return fail(error, tr("Unsupported list format or version."));
    if (!validItems(root["items"].toArray(), loading))
        return fail(error, tr("Invalid data in the list."));
    if (!target) return fail(error, tr("The destination list was closed."));
    const QJsonArray items = root["items"].toArray();
    int completed = 0;
    loading.setProgress(0, qMax(1, int(items.size())));
    loadItems(contents, items, loading, completed, qMax(1, int(items.size())));
    loading.setProgress(qMax(1, int(items.size())), qMax(1, int(items.size())));
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
    if (importInProgress) return fail(error, tr("Another import is already in progress."));
    QScopedValueRollback<bool> importing(importInProgress, true);
    QPointer<TabPlayer> target(player);
    LoadingDialog loading(player->window(), tr("Loading player…"));
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
        loading.refresh();
        if (!value.isObject()) return fail(error, tr("Invalid tab data."));
        const auto tab = value.toObject();
        if (!tab["name"].isString() || !tab["items"].isArray()
            || (tab.contains("listFile") && !tab["listFile"].isString())
            || !validItems(tab["items"].toArray(), loading)
            || (tab.contains("color") && !QColor(tab["color"].toString()).isValid()))
            return fail(error, tr("Invalid data in a player tab."));
    }

    if (!target) return fail(error, tr("The destination player was closed."));
    int total = player->count() + int(tabs.size());
    for (const auto &value : tabs) total += int(value.toObject()["items"].toArray().size());
    int completed = 0;
    loading.setProgress(0, total);
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
        loading.setProgress(++completed, total);
    }
    for (const auto &value : tabs) {
        const auto tab = value.toObject();
        auto *container = new Container(player);
        const QString name = tab["name"].toString();
        const int index = player->addTab(container, name);
        player->setTabToolTip(index, name);
        bar->setTabColor(index, QColor(tab["color"].toString()));
        auto *contents = qobject_cast<ContentsPlayer*>(container->widget());
        loadItems(contents, tab["items"].toArray(), loading, completed, total);
        contents->setListFileName(tab["listFile"].toString());
        loading.setProgress(++completed, total);
    }
    player->setCurrentIndex(currentIndex);
    blocker.unblock();
    player->setPlayerFileName(QFileInfo(filename).absoluteFilePath());
    return true;
}

