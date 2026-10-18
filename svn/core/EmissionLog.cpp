#include "core/EmissionLog.h"

#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QUuid>
#include <QDebug>

namespace {
QString csvField(QString value)
{
    value.replace('"', "\"\"");
    return QStringLiteral("\"") + value + QStringLiteral("\"");
}

QString timestamp(const QDateTime &dateTime)
{
    return dateTime.toString(Qt::ISODateWithMs);
}

QDateTime parseTimestamp(const QJsonObject &event, const QString &key)
{
    return QDateTime::fromString(event.value(key).toString(), Qt::ISODateWithMs);
}
}

EmissionLog::EmissionLog(QObject *parent) : QObject(parent)
{
    const QString appDataDirectory=QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation);
    m_directory=QDir(QCoreApplication::applicationDirPath())
                    .filePath(QStringLiteral("log"));
    if (!QDir().mkpath(m_directory))
        qWarning() << "Could not create the emission log directory:" << m_directory;

    // Preserve logs created by earlier versions in AppData.
    const QString migrationMarkerPath=QDir(m_directory).filePath(
        QStringLiteral(".appdata-migrated"));
    if (!QFileInfo::exists(migrationMarkerPath)) {
        const QStringList previousDirectories={
            QDir(appDataDirectory).filePath(QStringLiteral("log")),
            QDir(appDataDirectory).filePath(QStringLiteral("emission-log"))
        };
        bool migrationSucceeded=true;
        for (const QString &previousPath : previousDirectories) {
            const QDir previousDirectory(previousPath);
            const QStringList previousFiles=previousDirectory.entryList(
                {QStringLiteral("*.jsonl")},QDir::Files);
            for (const QString &name : previousFiles) {
                const QString destination=QDir(m_directory).filePath(name);
                const QString source=previousDirectory.filePath(name);
                if (!QFileInfo::exists(destination)) {
                    if (!QFile::copy(source,destination)) {
                        qWarning() << "Could not migrate emission log:" << name;
                        migrationSucceeded=false;
                    }
                    continue;
                }

                QFile sourceFile(source);
                QFile destinationFile(destination);
                if (!sourceFile.open(QIODevice::ReadOnly | QIODevice::Text)
                    || !destinationFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
                    qWarning() << "Could not merge emission log:" << name;
                    migrationSucceeded=false;
                    continue;
                }
                QSet<QByteArray> existingLines;
                while (!destinationFile.atEnd())
                    existingLines.insert(destinationFile.readLine().trimmed());
                destinationFile.seek(destinationFile.size());
                while (!sourceFile.atEnd()) {
                    const QByteArray line=sourceFile.readLine().trimmed();
                    if (line.isEmpty() || existingLines.contains(line))
                        continue;
                    const QByteArray output=line+QByteArrayLiteral("\n");
                    if (destinationFile.write(output)!=output.size()) {
                        migrationSucceeded=false;
                        break;
                    }
                    existingLines.insert(line);
                }
            }
        }
        if (migrationSucceeded) {
            QFile marker(migrationMarkerPath);
            if (!marker.open(QIODevice::WriteOnly | QIODevice::Text)
                || marker.write("done")!=4)
                qWarning() << "Could not create the emission log migration marker.";
        }
    }
    recoverUnfinishedRecords();
}

QString EmissionLog::fileForDate(const QDate &date) const
{
    return QDir(m_directory).filePath(date.toString(QStringLiteral("yyyy-MM-dd"))
                                      + QStringLiteral(".jsonl"));
}

void EmissionLog::trackStarted(const QObject *engine, const QString &source,
                               const QString &title, const QString &path)
{
    if (!engine)
        return;
    trackFinished(engine,tr("Interrupted"),tr("A new audio item started."));
    m_currentByEngine.insert(engine,startPlayback(source,title,path));
}

void EmissionLog::trackFinished(const QObject *engine, const QString &result,
                                const QString &details)
{
    const auto it=m_currentByEngine.find(engine);
    if (it==m_currentByEngine.end())
        return;
    const QString id=it.value();
    m_currentByEngine.erase(it);
    finishPlayback(id,result,details);
}

bool EmissionLog::appendEvent(const QDate &date, const QJsonObject &event) const
{
    QFile file(fileForDate(date));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << "Could not open the emission log:" << file.fileName() << file.errorString();
        return false;
    }
    const QByteArray line=QJsonDocument(event).toJson(QJsonDocument::Compact)+QByteArrayLiteral("\n");
    if (file.write(line)!=line.size()) {
        qWarning() << "Could not write the emission log:" << file.fileName() << file.errorString();
        return false;
    }
    file.flush();
    return file.error()==QFileDevice::NoError;
}

QString EmissionLog::startPlayback(const QString &source, const QString &title,
                                   const QString &path)
{
    const QDateTime now=QDateTime::currentDateTime();
    EmissionRecord record;
    record.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    record.startedAt=now;
    record.source=source;
    record.title=title;
    record.path=path;
    record.result=tr("Playing");

    QJsonObject event;
    event.insert(QStringLiteral("type"),QStringLiteral("start"));
    event.insert(QStringLiteral("id"),record.id);
    event.insert(QStringLiteral("startedAt"),timestamp(now));
    event.insert(QStringLiteral("source"),source);
    event.insert(QStringLiteral("title"),title);
    event.insert(QStringLiteral("path"),path);
    if (appendEvent(now.date(),event))
        m_activeRecords.insert(record.id,record);
    emit recordsChanged();
    return record.id;
}

void EmissionLog::finishPlayback(const QString &id, const QString &result,
                                 const QString &details)
{
    auto it=m_activeRecords.find(id);
    if (it==m_activeRecords.end())
        return;
    const QDateTime now=QDateTime::currentDateTime();
    QJsonObject event;
    event.insert(QStringLiteral("type"),QStringLiteral("finish"));
    event.insert(QStringLiteral("id"),id);
    event.insert(QStringLiteral("finishedAt"),timestamp(now));
    event.insert(QStringLiteral("result"),result);
    event.insert(QStringLiteral("details"),details);
    if (!appendEvent(it->startedAt.date(),event))
        return;
    m_activeRecords.erase(it);
    emit recordsChanged();
}

QVector<EmissionRecord> EmissionLog::recordsBetween(const QDate &from, const QDate &to) const
{
    QVector<EmissionRecord> records;
    if (!from.isValid() || !to.isValid() || from>to)
        return records;

    QHash<QString,int> rowById;
    for (QDate date=from; date<=to; date=date.addDays(1)) {
        QFile file(fileForDate(date));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        while (!file.atEnd()) {
            const QJsonDocument document=QJsonDocument::fromJson(file.readLine().trimmed());
            if (!document.isObject())
                continue;
            const QJsonObject event=document.object();
            const QString id=event.value(QStringLiteral("id")).toString();
            if (event.value(QStringLiteral("type")).toString()==QLatin1String("start")) {
                EmissionRecord record;
                record.id=id;
                record.startedAt=parseTimestamp(event,QStringLiteral("startedAt"));
                record.source=event.value(QStringLiteral("source")).toString();
                record.title=event.value(QStringLiteral("title")).toString();
                record.path=event.value(QStringLiteral("path")).toString();
                record.result=tr("Playing");
                rowById.insert(id,records.size());
                records.append(record);
            } else if (event.value(QStringLiteral("type")).toString()==QLatin1String("finish")
                       && rowById.contains(id)) {
                EmissionRecord &record=records[rowById.value(id)];
                record.finishedAt=parseTimestamp(event,QStringLiteral("finishedAt"));
                record.result=event.value(QStringLiteral("result")).toString();
                record.details=event.value(QStringLiteral("details")).toString();
            }
        }
    }
    return records;
}

bool EmissionLog::clearHistory(QString *error)
{
    const QDir directory(m_directory);
    const QStringList files=directory.entryList({QStringLiteral("*.jsonl")},QDir::Files);
    QStringList failures;
    for (const QString &name : files) {
        if (!QFile::remove(directory.filePath(name)))
            failures.append(name);
    }

    // The corresponding start records may have been removed, so do not append
    // orphan finish events when currently playing items complete.
    m_activeRecords.clear();
    m_currentByEngine.clear();
    emit recordsChanged();

    if (!failures.isEmpty()) {
        if (error)
            *error=tr("Could not delete log files: %1").arg(failures.join(QStringLiteral(", ")));
        return false;
    }
    return true;
}

bool EmissionLog::exportCsv(const QString &filename, const QDate &from, const QDate &to,
                            const QString &searchText, QString *error) const
{
    QSaveFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error) *error=file.errorString();
        return false;
    }
    file.write("Start;End;Source;Title;File;Result;Details\r\n");
    const QString needle=searchText.trimmed();
    for (const EmissionRecord &record : recordsBetween(from,to)) {
        if (!needle.isEmpty()
            && !record.title.contains(needle,Qt::CaseInsensitive)
            && !record.path.contains(needle,Qt::CaseInsensitive)
            && !record.source.contains(needle,Qt::CaseInsensitive)
            && !record.result.contains(needle,Qt::CaseInsensitive))
            continue;
        const QStringList columns={timestamp(record.startedAt),
            record.finishedAt.isValid() ? timestamp(record.finishedAt) : QString(),
            record.source,record.title,record.path,record.result,record.details};
        QStringList escaped;
        for (const QString &column : columns)
            escaped.append(csvField(column));
        file.write((escaped.join(';')+QStringLiteral("\r\n")).toUtf8());
    }
    if (!file.commit()) {
        if (error) *error=file.errorString();
        return false;
    }
    return true;
}

void EmissionLog::recoverUnfinishedRecords()
{
    const QDir directory(m_directory);
    const QStringList files=directory.entryList({QStringLiteral("*.jsonl")},QDir::Files);
    QHash<QString,QDate> active;
    for (const QString &name : files) {
        QFile file(directory.filePath(name));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        while (!file.atEnd()) {
            const QJsonDocument document=QJsonDocument::fromJson(file.readLine().trimmed());
            if (!document.isObject())
                continue;
            const QJsonObject event=document.object();
            const QString id=event.value(QStringLiteral("id")).toString();
            if (event.value(QStringLiteral("type")).toString()==QLatin1String("start"))
                active.insert(id,parseTimestamp(event,QStringLiteral("startedAt")).date());
            else if (event.value(QStringLiteral("type")).toString()==QLatin1String("finish"))
                active.remove(id);
        }
    }
    for (auto it=active.cbegin();it!=active.cend();++it) {
        QJsonObject event;
        event.insert(QStringLiteral("type"),QStringLiteral("finish"));
        event.insert(QStringLiteral("id"),it.key());
        event.insert(QStringLiteral("finishedAt"),timestamp(QDateTime::currentDateTime()));
        event.insert(QStringLiteral("result"),tr("Interrupted"));
        event.insert(QStringLiteral("details"),tr("Radit closed before playback completed."));
        appendEvent(it.value(),event);
    }
}
