/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/ElevenLabsSettingsDialog.h"

#include "core/MediaManager.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"

#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QLocale>
#include <QSettings>
#include <QShortcut>
#include <QTemporaryDir>
#include <QTime>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWindow>
#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif

namespace {
QByteArray protectApiKey(const QString &key)
{
    const QByteArray plain = key.toUtf8();
#ifdef Q_OS_WIN
    if (plain.isEmpty()) return {};
    DATA_BLOB input{};
    input.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(plain.constData()));
    input.cbData = static_cast<DWORD>(plain.size());
    DATA_BLOB encrypted{};
    if (!CryptProtectData(&input, L"Radit ElevenLabs API key", nullptr, nullptr, nullptr,
                          CRYPTPROTECT_UI_FORBIDDEN, &encrypted))
        return {};
    const QByteArray result(reinterpret_cast<const char *>(encrypted.pbData),
                             static_cast<qsizetype>(encrypted.cbData));
    LocalFree(encrypted.pbData);
    return result;
#else
    return plain;
#endif
}

QString unprotectApiKey(const QByteArray &stored)
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
}

ElevenLabsSettingsDialog::ElevenLabsSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("OnlineRadioDialog");
    setWindowTitle(tr("ElevenLabs settings"));
    setWindowIcon(QIcon(":/icons/radit.ico"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    resize(510, 315);
    m_network = new QNetworkAccessManager(this);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_titleBar = new Frame(this);
    m_titleBar->setObjectName("framebarra");
    m_titleBar->setFixedHeight(25);
    m_titleBar->installEventFilter(this);
    auto *barLayout = new QHBoxLayout(m_titleBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->setSpacing(0);
    auto *title = new Label(m_titleBar);
    title->setObjectName("PanelTitle");
    title->setText(windowTitle());
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    barLayout->addWidget(title);
    barLayout->addStretch();
    auto *close = new Button(m_titleBar);
    close->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    close->setFixedSize(15, 15);
    close->SetIcon("Close-hover.svg");
    close->setIconSize(QSize(15, 15));
    close->setToolTip(tr("Close ElevenLabs settings"));
    barLayout->addWidget(close);
    connect(close, &Button::clicked, this, &QDialog::reject);
    root->addWidget(m_titleBar);

    auto *content = new QWidget(this);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(12, 10, 12, 10);
    contentLayout->setSpacing(9);
    auto *form = new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);

    m_apiKey = new QLineEdit(content);
    m_apiKey->setEchoMode(QLineEdit::Password);
    m_apiKey->setPlaceholderText(tr("ElevenLabs API key"));
    auto *keyRow = new QHBoxLayout;
    keyRow->setSpacing(6);
    keyRow->addWidget(m_apiKey, 1);
    auto *showKey = new Button(content);
    showKey->setText(tr("Show"));
    showKey->setCheckable(true);
    showKey->setFixedSize(58, 24);
    keyRow->addWidget(showKey);
    connect(showKey, &Button::toggled, this, [this](bool visible) {
        m_apiKey->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });
    form->addRow(tr("API key"), keyRow);

    m_voiceId = new QLineEdit(content);
    m_voiceId->setPlaceholderText(tr("Voice ID from your ElevenLabs account"));
    form->addRow(tr("Voice ID"), m_voiceId);

    m_model = new QComboBox(content);
    m_model->setObjectName("Combo");
    m_model->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_model->addItem("eleven_multilingual_v2", "eleven_multilingual_v2");
    m_model->addItem("eleven_flash_v2_5", "eleven_flash_v2_5");
    m_model->addItem("eleven_v3", "eleven_v3");
    form->addRow(tr("Model"), m_model);
    contentLayout->addLayout(form);

    m_testStatus = new QLabel(tr("A short Spanish sample will be generated and played."), content);
    m_testStatus->setWordWrap(true);
    contentLayout->addWidget(m_testStatus);

    auto *accountRow = new QHBoxLayout;
    accountRow->setSpacing(8);
    auto *accountCaption = new QLabel(tr("Account"), content);
    m_accountName = new QLabel(tr("Not checked"), content);
    m_accountName->setObjectName("ElevenLabsAccountName");
    accountRow->addWidget(accountCaption);
    accountRow->addWidget(m_accountName, 1);
    contentLayout->addLayout(accountRow);

    auto *creditsRow = new QHBoxLayout;
    creditsRow->setSpacing(8);
    auto *creditsCaption = new QLabel(tr("Credits"), content);
    m_creditStatus = new QLabel(tr("Not checked"), content);
    m_creditStatus->setObjectName("ElevenLabsCredits");
    m_refreshAccountButton = new Button(content);
    m_refreshAccountButton->setText(tr("Refresh"));
    m_refreshAccountButton->setFixedSize(72, 24);
    creditsRow->addWidget(creditsCaption);
    creditsRow->addWidget(m_creditStatus, 1);
    creditsRow->addWidget(m_refreshAccountButton);
    contentLayout->addLayout(creditsRow);

    m_accountStatus = new QLabel(content);
    m_accountStatus->setObjectName("ElevenLabsAccountStatus");
    m_accountStatus->setWordWrap(true);
    contentLayout->addWidget(m_accountStatus);

    auto *buttons = new QHBoxLayout;
    buttons->setSpacing(8);
    m_testButton = new Button(content);
    m_testButton->setText(tr("Test voice"));
    m_testButton->setFixedSize(88, 24);
    buttons->addWidget(m_testButton);
    buttons->addStretch();
    auto *cancel = new Button(content);
    cancel->setText(tr("Cancel"));
    auto *save = new Button(content);
    save->setText(tr("Save"));
    cancel->setFixedSize(70, 24);
    save->setFixedSize(70, 24);
    buttons->addWidget(cancel);
    buttons->addWidget(save);
    contentLayout->addLayout(buttons);
    root->addWidget(content);

    QSettings stored(QSettings::IniFormat, QSettings::UserScope, "Radit", "Radit");
    stored.beginGroup("ElevenLabs");
    m_apiKey->setText(unprotectApiKey(stored.value("apiKey").toByteArray()));
    m_voiceId->setText(stored.value("voiceId").toString());
    const int modelIndex = m_model->findData(
        stored.value("model", "eleven_multilingual_v2").toString());
    m_model->setCurrentIndex(modelIndex >= 0 ? modelIndex : 0);
    stored.endGroup();

    connect(cancel, &Button::clicked, this, &QDialog::reject);
    connect(m_testButton, &Button::clicked, this, &ElevenLabsSettingsDialog::testVoice);
    connect(m_refreshAccountButton, &Button::clicked,
            this, &ElevenLabsSettingsDialog::refreshAccountInfo);
    connect(save, &Button::clicked, this, [this]() {
        if (saveSettings()) accept();
    });
    auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escape, &QShortcut::activated, this, &QDialog::reject);
    refreshAccountInfo();
}

ElevenLabsSettingsDialog::~ElevenLabsSettingsDialog()
{
    clearPreview();
}

void ElevenLabsSettingsDialog::clearPreview()
{
    if (m_previewMedia) {
        m_previewMedia->stop();
        delete m_previewMedia;
        m_previewMedia = nullptr;
    }
    delete m_previewDirectory;
    m_previewDirectory = nullptr;
}

void ElevenLabsSettingsDialog::testVoice()
{
    const QString apiKey = m_apiKey->text().trimmed();
    const QString voiceId = m_voiceId->text().trimmed();
    if (apiKey.isEmpty() || voiceId.isEmpty()) {
        m_testStatus->setText(tr("Enter the API key and Voice ID first."));
        return;
    }

    clearPreview();
    m_testButton->setEnabled(false);
    m_testStatus->setText(tr("Connecting to ElevenLabs…"));

    const QString time = QLocale(QLocale::Spanish, QLocale::Spain)
                             .toString(QTime::currentTime(), "H:mm");
    const QString sample = tr("Esta es una prueba de voz de Radit. Son las %1. "
                              "La temperatura es de veinte grados y la humedad es del cincuenta por ciento.")
                               .arg(time);
    QUrl url(QStringLiteral("https://api.elevenlabs.io/v1/text-to-speech/") +
             QString::fromLatin1(QUrl::toPercentEncoding(voiceId)));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("output_format"), QStringLiteral("mp3_44100_128"));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    request.setRawHeader("xi-api-key", apiKey.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject payload;
    payload.insert(QStringLiteral("text"), sample);
    payload.insert(QStringLiteral("model_id"), m_model->currentData().toString());

    QNetworkReply *reply = m_network->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_testButton->setEnabled(true);
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray audio = reply->readAll();
        const QString networkError = reply->errorString();
        QString apiMessage;
        const QJsonDocument errorDocument = QJsonDocument::fromJson(audio);
        if (errorDocument.isObject()) {
            const QJsonValue detail = errorDocument.object().value(QStringLiteral("detail"));
            if (detail.isObject())
                apiMessage = detail.toObject().value(QStringLiteral("message")).toString();
            else if (detail.isString())
                apiMessage = detail.toString();
        }
        reply->deleteLater();

        if (status < 200 || status >= 300 || audio.isEmpty()) {
            if (!apiMessage.isEmpty()) {
                m_testStatus->setText(tr("ElevenLabs error (HTTP %1): %2").arg(status).arg(apiMessage));
            } else if (status == 402) {
                m_testStatus->setText(tr("ElevenLabs returned HTTP 402: the account or API key has no credits available for this request."));
            } else if (status) {
                m_testStatus->setText(tr("ElevenLabs returned HTTP %1. Check the API key, Voice ID, model and account access.").arg(status));
            } else {
                m_testStatus->setText(tr("Could not connect to ElevenLabs: %1").arg(networkError));
            }
            return;
        }

        m_previewDirectory = new QTemporaryDir;
        if (!m_previewDirectory->isValid()) {
            clearPreview();
            m_testStatus->setText(tr("Radit could not create a temporary audio file."));
            return;
        }
        QFile file(m_previewDirectory->filePath(QStringLiteral("voice-test.mp3")));
        if (!file.open(QIODevice::WriteOnly) || file.write(audio) != audio.size()) {
            clearPreview();
            m_testStatus->setText(tr("Radit could not save the generated audio."));
            return;
        }
        file.close();

        m_previewMedia = new MediaManager(this);
        if (!m_previewMedia->setDevice(-1) || !m_previewMedia->loadFile(file.fileName())) {
            clearPreview();
            m_testStatus->setText(tr("The voice was generated, but Radit could not play the preview."));
            return;
        }
        connect(m_previewMedia, &MediaManager::playbackFinished, this, [this]() {
            clearPreview();
            m_testStatus->setText(tr("Voice test finished."));
        });
        connect(m_previewMedia, &MediaManager::playbackError, this, [this](const QString &) {
            clearPreview();
            m_testStatus->setText(tr("The voice was generated, but Radit could not play the preview."));
        });
        m_testStatus->setText(tr("Playing voice test…"));
        m_previewMedia->play();
    });
}

void ElevenLabsSettingsDialog::refreshAccountInfo()
{
    const QString apiKey = m_apiKey->text().trimmed();
    if (apiKey.isEmpty()) {
        m_accountName->setText(tr("—"));
        m_creditStatus->setText(tr("—"));
        m_accountStatus->setText(tr("Enter an API key to check account details."));
        return;
    }

    m_refreshAccountButton->setEnabled(false);
    m_accountStatus->setText(tr("Checking ElevenLabs account…"));
    QNetworkRequest request(QUrl(QStringLiteral("https://api.elevenlabs.io/v1/user")));
    request.setTransferTimeout(15000);
    request.setRawHeader("xi-api-key", apiKey.toUtf8());
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_refreshAccountButton->setEnabled(true);
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray response = reply->readAll();
        const QString networkError = reply->errorString();
        reply->deleteLater();

        const QJsonDocument document = QJsonDocument::fromJson(response);
        if (status < 200 || status >= 300 || !document.isObject()) {
            m_accountName->setText(tr("Unavailable"));
            m_creditStatus->setText(tr("Unavailable"));
            if (status == 401 || status == 403) {
                m_accountStatus->setText(tr("Checking usage access…"));
                QNetworkRequest subscriptionRequest(
                    QUrl(QStringLiteral("https://api.elevenlabs.io/v1/user/subscription")));
                subscriptionRequest.setTransferTimeout(15000);
                subscriptionRequest.setRawHeader("xi-api-key", m_apiKey->text().trimmed().toUtf8());
                QNetworkReply *subscriptionReply = m_network->get(subscriptionRequest);
                connect(subscriptionReply, &QNetworkReply::finished, this,
                        [this, subscriptionReply]() {
                    m_refreshAccountButton->setEnabled(true);
                    const int subscriptionStatus = subscriptionReply
                        ->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                    const QJsonDocument subscriptionDocument =
                        QJsonDocument::fromJson(subscriptionReply->readAll());
                    subscriptionReply->deleteLater();

                    if (subscriptionStatus >= 200 && subscriptionStatus < 300
                        && subscriptionDocument.isObject()) {
                        const QJsonObject subscription = subscriptionDocument.object();
                        const qint64 used = subscription.value(QStringLiteral("character_count"))
                                                .toVariant().toLongLong();
                        const qint64 limit = subscription.value(QStringLiteral("character_limit"))
                                                 .toVariant().toLongLong();
                        if (limit > 0) {
                            const QLocale locale;
                            m_creditStatus->setText(
                                tr("%1 used · %2 remaining of %3 characters")
                                    .arg(locale.toString(used),
                                         locale.toString(qMax<qint64>(0, limit - used)),
                                         locale.toString(limit)));
                            m_accountStatus->setText(
                                tr("Usage is available, but this key cannot read the account name. Enable user_read to show it."));
                            return;
                        }
                    }

                    m_accountStatus->setText(
                        tr("This API key lacks user_read permission. Edit it in ElevenLabs and enable account/user read access to show the account name and credits."));
                });
                return;
            } else if (status) {
                m_accountStatus->setText(tr("ElevenLabs account check failed (HTTP %1)." ).arg(status));
            } else {
                m_accountStatus->setText(tr("Could not connect to ElevenLabs: %1").arg(networkError));
            }
            return;
        }

        const QJsonObject user = document.object();
        const QString name = user.value(QStringLiteral("first_name")).toString().trimmed();
        const QString userId = user.value(QStringLiteral("user_id")).toString().trimmed();
        m_accountName->setText(!name.isEmpty() ? name : (!userId.isEmpty() ? userId : tr("Unknown")));

        const QJsonObject subscription = user.value(QStringLiteral("subscription")).toObject();
        const qint64 used = subscription.value(QStringLiteral("character_count")).toVariant().toLongLong();
        const qint64 limit = subscription.value(QStringLiteral("character_limit")).toVariant().toLongLong();
        if (limit > 0) {
            const qint64 remaining = qMax<qint64>(0, limit - used);
            const QLocale locale;
            m_creditStatus->setText(tr("%1 used · %2 remaining of %3 characters")
                                        .arg(locale.toString(used), locale.toString(remaining),
                                             locale.toString(limit)));
            m_accountStatus->setText(tr("Usage is reported by ElevenLabs as text characters; generated voice tests may consume them."));
        } else {
            m_creditStatus->setText(tr("Usage information unavailable"));
            m_accountStatus->setText(tr("ElevenLabs did not include a character limit for this account."));
        }
    });
}

bool ElevenLabsSettingsDialog::saveSettings()
{
    const QByteArray protectedKey = protectApiKey(m_apiKey->text().trimmed());
#ifdef Q_OS_WIN
    if (!m_apiKey->text().trimmed().isEmpty() && protectedKey.isEmpty()) {
        QMessageBox::warning(this, tr("ElevenLabs settings"),
                             tr("Windows could not securely save the API key."));
        return false;
    }
#endif
    QSettings stored(QSettings::IniFormat, QSettings::UserScope, "Radit", "Radit");
    stored.beginGroup("ElevenLabs");
    if (m_apiKey->text().trimmed().isEmpty())
        stored.remove("apiKey");
    else
        stored.setValue("apiKey", protectedKey);
    stored.setValue("voiceId", m_voiceId->text().trimmed());
    stored.setValue("model", m_model->currentData().toString());
    stored.endGroup();
    stored.sync();
    if (stored.status() != QSettings::NoError) {
        QMessageBox::warning(this, tr("ElevenLabs settings"),
                             tr("Radit could not save the ElevenLabs settings."));
        return false;
    }
    return true;
}

bool ElevenLabsSettingsDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->button() == Qt::LeftButton) {
                if (windowHandle() && windowHandle()->startSystemMove())
                    return true;
                m_dragging = true;
                m_dragOffset = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_dragging) {
            auto *mouse = static_cast<QMouseEvent *>(event);
            if (mouse->buttons() & Qt::LeftButton)
                move(mouse->globalPosition().toPoint() - m_dragOffset);
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
        }
    }
    return QDialog::eventFilter(watched, event);
}
