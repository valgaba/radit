#ifndef EMISSIONLOG_H
#define EMISSIONLOG_H

#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QVector>

struct EmissionRecord
{
    QString id;
    QDateTime startedAt;
    QDateTime finishedAt;
    QString source;
    QString title;
    QString path;
    QString result;
    QString details;
};

class EmissionLog final : public QObject
{
    Q_OBJECT
public:
    explicit EmissionLog(QObject *parent = nullptr);

    void trackStarted(const QObject *engine, const QString &source,
                      const QString &title, const QString &path);
    void trackFinished(const QObject *engine, const QString &result,
                       const QString &details = {});
    QString startPlayback(const QString &source, const QString &title, const QString &path);
    void finishPlayback(const QString &id, const QString &result, const QString &details = {});
    QVector<EmissionRecord> recordsBetween(const QDate &from, const QDate &to) const;
    bool clearHistory(QString *error = nullptr);
    bool exportCsv(const QString &filename, const QDate &from, const QDate &to,
                   const QString &searchText = {}, QString *error = nullptr) const;

signals:
    void recordsChanged();

private:
    QString m_directory;
    QHash<QString, EmissionRecord> m_activeRecords;
    QHash<const QObject *,QString> m_currentByEngine;
    QString fileForDate(const QDate &date) const;
    bool appendEvent(const QDate &date, const QJsonObject &event) const;
    void recoverUnfinishedRecords();
};

#endif // EMISSIONLOG_H
