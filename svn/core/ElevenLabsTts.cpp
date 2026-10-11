#include "core/ElevenLabsTts.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QCryptographicHash>
#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif

namespace {
QString decodeApiKey(const QByteArray &stored)
{
    if (stored.isEmpty()) return {};
#ifdef Q_OS_WIN
    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(stored.constData()));
    input.cbData = static_cast<DWORD>(stored.size());
    DATA_BLOB decrypted{};
    if (!CryptUnprotectData(&input, nullptr, nullptr, nullptr, nullptr,
                            CRYPTPROTECT_UI_FORBIDDEN, &decrypted))
        return {};
    const QString result = QString::fromUtf8(reinterpret_cast<const char *>(decrypted.pbData),
                                             static_cast<qsizetype>(decrypted.cbData));
    LocalFree(decrypted.pbData);
    return result;
#else
    return QString::fromUtf8(stored);
#endif
}

QNetworkAccessManager *networkManager()
{
    static QPointer<QNetworkAccessManager> manager;
    if (!manager && QCoreApplication::instance())
        manager = new QNetworkAccessManager(QCoreApplication::instance());
    return manager;
}
}

ElevenLabsTts::ElevenLabsTts(QObject *parent) : QObject(parent) {}

ElevenLabsTts *ElevenLabsTts::instance()
{
    static QPointer<ElevenLabsTts> service;
    if (!service && QCoreApplication::instance())
        service = new ElevenLabsTts(QCoreApplication::instance());
    return service;
}

void ElevenLabsTts::generate(const QString &text, QObject *context, Callback callback)
{
    const auto finish = [guard = QPointer<QObject>(context), callback](const QString &path,
                                                                       const QString &error) {
        if (!guard || !callback) return;
        QTimer::singleShot(0, guard.data(), [guard, callback, path, error]() {
            if (guard) callback(path, error);
        });
    };
    const QString cleanText = text.trimmed();
    if (cleanText.isEmpty()) { finish({}, {}); return; }

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Radit", "Radit");
    settings.beginGroup("ElevenLabs");
    const QString apiKey = decodeApiKey(settings.value("apiKey").toByteArray());
    const QString voiceId = settings.value("voiceId").toString().trimmed();
    const QString model = settings.value("model", "eleven_multilingual_v2").toString();
    settings.endGroup();
    if (apiKey.isEmpty() || voiceId.isEmpty()) {
        finish({}, tr("Configure the ElevenLabs API key and Voice ID first."));
        return;
    }

    const QByteArray cacheKey = QCryptographicHash::hash(
        (voiceId + QLatin1Char('\n') + model + QLatin1Char('\n') + cleanText).toUtf8(),
        QCryptographicHash::Sha256).toHex();
    const QString cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                                   + QStringLiteral("/elevenlabs");
    QDir().mkpath(cacheDirectory);
    const QString cachedPath = cacheDirectory + QLatin1Char('/')
                               + QString::fromLatin1(cacheKey) + QStringLiteral(".mp3");
    if (QFile::exists(cachedPath)) { finish(cachedPath, {}); return; }

    QNetworkAccessManager *manager = networkManager();
    if (!manager) { finish({}, tr("The network service is unavailable.")); return; }
    QUrl url(QStringLiteral("https://api.elevenlabs.io/v1/text-to-speech/")
             + QString::fromLatin1(QUrl::toPercentEncoding(voiceId)));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("output_format"), QStringLiteral("mp3_44100_128"));
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    request.setRawHeader("xi-api-key", apiKey.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject payload;
    payload.insert(QStringLiteral("text"), cleanText);
    payload.insert(QStringLiteral("model_id"), model);
    QNetworkReply *reply = manager->post(request,
        QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this,
            [reply, cachedPath, finish]() {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const QString networkError = reply->errorString();
        QString error;
        if (status < 200 || status >= 300 || body.isEmpty()) {
            const QJsonDocument document = QJsonDocument::fromJson(body);
            if (document.isObject()) {
                const QJsonValue detail = document.object().value(QStringLiteral("detail"));
                if (detail.isObject())
                    error = detail.toObject().value(QStringLiteral("message")).toString();
                else if (detail.isString())
                    error = detail.toString();
            }
            if (error.isEmpty()) error = status ? tr("ElevenLabs returned HTTP %1.").arg(status)
                                                : networkError;
            reply->deleteLater();
            finish({}, error);
            return;
        }
        QFile file(cachedPath);
        if (!file.open(QIODevice::WriteOnly) || file.write(body) != body.size()) {
            error = tr("Radit could not save the generated voice clip.");
            reply->deleteLater();
            finish({}, error);
            return;
        }
        file.close();
        reply->deleteLater();
        finish(cachedPath, {});
    });
}
