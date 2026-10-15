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
#include <QJsonArray>
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
#include <QSignalBlocker>
#include <QShortcut>
#include <QTemporaryDir>
#include <QTime>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWindow>
#include <QXmlStreamWriter>
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
    setWindowTitle(tr("AI voice settings"));
    setWindowIcon(QIcon(":/icons/radit.ico"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    resize(510, 385);
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
    close->setToolTip(tr("Close AI voice settings"));
    barLayout->addWidget(close);
    connect(close, &Button::clicked, this, &QDialog::reject);
    root->addWidget(m_titleBar);

    auto *content = new QWidget(this);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(12, 10, 12, 10);
    contentLayout->setSpacing(9);
    m_form = new QFormLayout;
    m_form->setHorizontalSpacing(12);
    m_form->setVerticalSpacing(8);
    m_provider = new QComboBox(content);
    m_provider->setObjectName("Combo");
    m_provider->addItem(tr("ElevenLabs"), "elevenlabs");
    m_provider->addItem(tr("Azure Speech"), "azure");
    m_form->addRow(tr("Provider"), m_provider);

    m_apiKey = new QLineEdit(content);
    m_apiKey->setEchoMode(QLineEdit::Password);
    m_apiKey->setPlaceholderText(tr("Provider API key"));
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
    m_apiKeyLabel = new QLabel(tr("API key"), content);
    m_form->addRow(m_apiKeyLabel, keyRow);

    m_voiceId = new QLineEdit(content);
    m_voiceId->setPlaceholderText(tr("Voice ID from your ElevenLabs account"));
    m_form->addRow(tr("Voice ID"), m_voiceId);
    m_voiceName = new QLabel(tr("Not checked"), content);
    m_voiceName->setObjectName("ElevenLabsVoiceName");
    m_voiceName->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_form->addRow(tr("Voice name"), m_voiceName);

    m_model = new QComboBox(content);
    m_model->setObjectName("Combo");
    m_model->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_model->addItem("eleven_multilingual_v2", "eleven_multilingual_v2");
    m_model->addItem("eleven_flash_v2_5", "eleven_flash_v2_5");
    m_model->addItem("eleven_v3", "eleven_v3");
    m_form->addRow(tr("Model"), m_model);

    m_azureRegion = new QComboBox(content);
    m_azureRegion->setObjectName("Combo");
    m_azureRegion->setEditable(true);
    m_azureRegion->addItems({"westeurope", "northeurope", "francecentral", "germanywestcentral",
                             "uksouth", "eastus", "westus2", "centralus", "brazilsouth",
                             "eastasia", "southeastasia", "australiaeast"});
    m_azureRegion->setCurrentText("westeurope");
    m_form->addRow(tr("Azure region"), m_azureRegion);
    auto *azureVoiceRow = new QHBoxLayout;
    azureVoiceRow->setSpacing(6);
    m_azureVoice = new QComboBox(content);
    m_azureVoice->setObjectName("Combo");
    m_azureVoice->setEditable(true);
    m_azureVoice->setInsertPolicy(QComboBox::NoInsert);
    m_azureVoice->setPlaceholderText(tr("Voice name, e.g. es-ES-ElviraNeural"));
    azureVoiceRow->addWidget(m_azureVoice, 1);
    m_refreshAzureVoicesButton = new Button(content);
    m_refreshAzureVoicesButton->setText(tr("Refresh"));
    m_refreshAzureVoicesButton->setFixedSize(72, 24);
    azureVoiceRow->addWidget(m_refreshAzureVoicesButton);
    m_form->addRow(tr("Azure voice"), azureVoiceRow);
    contentLayout->addLayout(m_form);

    // Keep row visibility provider-specific while sharing the key and test controls.
    m_form->setRowVisible(2, false);
    m_form->setRowVisible(3, false);
    m_form->setRowVisible(4, false);
    m_form->setRowVisible(5, false);
    m_form->setRowVisible(6, false);

    m_testStatus = new QLabel(tr("A short Spanish sample will be generated and played."), content);
    m_testStatus->setWordWrap(true);
    contentLayout->addWidget(m_testStatus);

    m_accountPanel = new QWidget(content);
    auto *accountLayout = new QVBoxLayout(m_accountPanel);
    accountLayout->setContentsMargins(0, 0, 0, 0);
    accountLayout->setSpacing(6);
    auto *accountRow = new QHBoxLayout;
    accountRow->setSpacing(8);
    auto *accountCaption = new QLabel(tr("Account"), content);
    m_accountName = new QLabel(tr("Not checked"), content);
    m_accountName->setObjectName("ElevenLabsAccountName");
    accountRow->addWidget(accountCaption);
    accountRow->addWidget(m_accountName, 1);
    accountLayout->addLayout(accountRow);

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
    accountLayout->addLayout(creditsRow);

    m_accountStatus = new QLabel(content);
    m_accountStatus->setObjectName("ElevenLabsAccountStatus");
    m_accountStatus->setWordWrap(true);
    accountLayout->addWidget(m_accountStatus);
    contentLayout->addWidget(m_accountPanel);

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
    ProviderDraft eleven;
    eleven.apiKey = unprotectApiKey(stored.value("apiKey").toByteArray());
    eleven.voiceId = stored.value("voiceId").toString();
    eleven.model = stored.value("model", "eleven_multilingual_v2").toString();
    stored.endGroup();
    m_providerDrafts.insert("elevenlabs", eleven);
    stored.beginGroup("AzureSpeech");
    ProviderDraft azure;
    azure.apiKey = unprotectApiKey(stored.value("apiKey").toByteArray());
    azure.region = stored.value("region", "westeurope").toString();
    azure.voiceName = stored.value("voiceName", "es-ES-ElviraNeural").toString();
    azure.voiceLocale = stored.value("voiceLocale", "es-ES").toString();
    stored.endGroup();
    m_providerDrafts.insert("azure", azure);
    int providerIndex = m_provider->findData(stored.value("AIVoices/provider", "elevenlabs").toString());
    if (providerIndex < 0) providerIndex = 0;
    m_provider->setCurrentIndex(providerIndex);
    m_displayedProvider = selectedProvider();
    applyProviderUi();

    connect(cancel, &Button::clicked, this, &QDialog::reject);
    connect(m_testButton, &Button::clicked, this, &ElevenLabsSettingsDialog::testVoice);
    connect(m_refreshAccountButton, &Button::clicked,
            this, &ElevenLabsSettingsDialog::refreshAccountInfo);
    connect(m_voiceId, &QLineEdit::editingFinished,
            this, &ElevenLabsSettingsDialog::refreshVoiceName);
    connect(m_apiKey, &QLineEdit::editingFinished,
            this, &ElevenLabsSettingsDialog::refreshVoiceName);
    connect(m_provider, &QComboBox::currentIndexChanged, this, [this](int) {
        captureProviderDraft();
        m_displayedProvider = selectedProvider();
        applyProviderUi();
    });
    connect(m_azureRegion, &QComboBox::currentTextChanged, this, [this]() {
        ProviderDraft &draft = m_providerDrafts["azure"];
        draft.region = m_azureRegion->currentText().trimmed();
        const QString voiceName = azureVoiceName();
        const QString voiceLocale = azureVoiceLocale();
        const QSignalBlocker blocker(m_azureVoice);
        m_azureVoice->clear();
        m_azureVoice->setEditText(voiceName);
        draft.voiceName = voiceName;
        draft.voiceLocale = voiceLocale;
    });
    connect(m_azureVoice, &QComboBox::currentTextChanged, this, [this](const QString &) {
        m_providerDrafts["azure"].voiceName = azureVoiceName();
        m_providerDrafts["azure"].voiceLocale = azureVoiceLocale();
    });
    connect(m_refreshAzureVoicesButton, &Button::clicked,
            this, &ElevenLabsSettingsDialog::refreshAzureVoices);
    connect(save, &Button::clicked, this, [this]() {
        if (saveSettings()) accept();
    });
    auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escape, &QShortcut::activated, this, &QDialog::reject);
    if (selectedProvider() == "elevenlabs") {
        refreshAccountInfo();
        refreshVoiceName();
    } else {
        m_azureVoice->setEditText(azure.voiceName);
    }
}

QString ElevenLabsSettingsDialog::selectedProvider() const
{
    return m_provider ? m_provider->currentData().toString() : QStringLiteral("elevenlabs");
}

QString ElevenLabsSettingsDialog::azureVoiceName() const
{
    const QString id = m_azureVoice->currentData().toString().trimmed();
    return id.isEmpty() ? m_azureVoice->currentText().trimmed() : id;
}

QString ElevenLabsSettingsDialog::azureVoiceLocale() const
{
    const QString locale = m_azureVoice->currentData(Qt::UserRole + 1).toString().trimmed();
    return locale.isEmpty() ? QStringLiteral("es-ES") : locale;
}

void ElevenLabsSettingsDialog::captureProviderDraft()
{
    const QString provider = m_displayedProvider.isEmpty() ? selectedProvider() : m_displayedProvider;
    ProviderDraft &draft = m_providerDrafts[provider];
    draft.apiKey = m_apiKey->text().trimmed();
    if (provider == QLatin1String("elevenlabs")) {
        draft.voiceId = m_voiceId->text().trimmed();
        draft.model = m_model->currentData().toString();
    } else {
        draft.region = m_azureRegion->currentText().trimmed();
        draft.voiceName = azureVoiceName();
        draft.voiceLocale = azureVoiceLocale();
    }
}

void ElevenLabsSettingsDialog::applyProviderUi()
{
    const bool elevenLabs = selectedProvider() == QLatin1String("elevenlabs");
    const ProviderDraft draft = m_providerDrafts.value(selectedProvider());
    m_apiKey->setText(draft.apiKey);
    m_apiKeyLabel->setText(elevenLabs ? tr("API key") : tr("Subscription key"));
    m_apiKey->setPlaceholderText(elevenLabs ? tr("ElevenLabs API key")
                                            : tr("Azure Speech subscription key"));
    m_form->setRowVisible(2, elevenLabs);
    m_form->setRowVisible(3, elevenLabs);
    m_form->setRowVisible(4, elevenLabs);
    m_form->setRowVisible(5, !elevenLabs);
    m_form->setRowVisible(6, !elevenLabs);
    m_voiceId->setText(draft.voiceId);
    m_voiceName->setText(draft.voiceId.isEmpty() ? tr("Enter a Voice ID to look up its name.") : tr("Not checked"));
    const int modelIndex = m_model->findData(draft.model.isEmpty()
                                                  ? QStringLiteral("eleven_multilingual_v2")
                                                  : draft.model);
    m_model->setCurrentIndex(modelIndex >= 0 ? modelIndex : 0);
    m_azureRegion->setCurrentText(draft.region.isEmpty() ? QStringLiteral("westeurope") : draft.region);
    const QSignalBlocker azureVoiceSignals(m_azureVoice);
    m_azureVoice->clear();
    m_azureVoice->setEditText(draft.voiceName.isEmpty()
                                  ? QStringLiteral("es-ES-ElviraNeural") : draft.voiceName);
    m_accountPanel->setVisible(elevenLabs);
    m_testStatus->setText(elevenLabs
        ? tr("A short Spanish sample will be generated and played.")
        : tr("Azure Speech will generate a short Spanish sample and play it."));
    if (elevenLabs) {
        refreshAccountInfo();
        refreshVoiceName();
    } else {
        m_accountName->clear();
        m_creditStatus->clear();
        m_accountStatus->clear();
    }
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
    if (selectedProvider() == QLatin1String("azure")) {
        const QString region = m_azureRegion->currentText().trimmed();
        const QString voice = azureVoiceName();
        if (apiKey.isEmpty() || region.isEmpty() || voice.isEmpty()) {
            m_testStatus->setText(tr("Enter the Azure subscription key, region and voice first."));
            return;
        }
        clearPreview();
        m_testButton->setEnabled(false);
        m_testStatus->setText(tr("Connecting to Azure Speech…"));
        const QString sample = tr("Esta es una prueba de voz de Radit. Son las %1. La temperatura es de veinte grados.")
                                   .arg(QLocale(QLocale::Spanish, QLocale::Spain)
                                            .toString(QTime::currentTime(), "H:mm"));
        QByteArray ssml;
        QXmlStreamWriter xml(&ssml);
        xml.writeStartDocument();
        xml.writeStartElement(QStringLiteral("speak"));
        xml.writeAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
        xml.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://www.w3.org/2001/10/synthesis"));
        xml.writeAttribute(QStringLiteral("xmlns:mstts"), QStringLiteral("https://www.w3.org/2001/mstts"));
        xml.writeAttribute(QStringLiteral("xml:lang"), QStringLiteral("es-ES"));
        xml.writeStartElement(QStringLiteral("voice"));
        xml.writeAttribute(QStringLiteral("name"), voice);
        xml.writeCharacters(sample);
        xml.writeEndElement();
        xml.writeEndElement();
        xml.writeEndDocument();

        const QUrl url(QStringLiteral("https://") + region
                       + QStringLiteral(".tts.speech.microsoft.com/cognitiveservices/v1"));
        QNetworkRequest request(url);
        request.setTransferTimeout(30000);
        request.setRawHeader("Ocp-Apim-Subscription-Key", apiKey.toUtf8());
        request.setRawHeader("X-Microsoft-OutputFormat", "audio-24khz-96kbitrate-mono-mp3");
        request.setRawHeader("User-Agent", "Radit");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/ssml+xml");
        QNetworkReply *reply = m_network->post(request, ssml);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            m_testButton->setEnabled(true);
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray response = reply->readAll();
            const QString networkError = reply->errorString();
            reply->deleteLater();
            if (status < 200 || status >= 300 || response.isEmpty()) {
                QString details = QString::fromUtf8(response).trimmed();
                if (details.size() > 240) details.truncate(240);
                m_testStatus->setText(status
                    ? tr("Azure Speech returned HTTP %1. Check the key, region and voice. %2").arg(status).arg(details)
                    : tr("Could not connect to Azure Speech: %1").arg(networkError));
                return;
            }
            playPreview(response);
        });
        return;
    }
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

        playPreview(audio);
    });
}

void ElevenLabsSettingsDialog::playPreview(const QByteArray &audio)
{
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
}

void ElevenLabsSettingsDialog::refreshAccountInfo()
{
    if (selectedProvider() != QLatin1String("elevenlabs")) return;
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

void ElevenLabsSettingsDialog::refreshVoiceName()
{
    if (selectedProvider() != QLatin1String("elevenlabs")) return;
    const QString apiKey = m_apiKey->text().trimmed();
    const QString voiceId = m_voiceId->text().trimmed();
    const quint64 lookupSerial = ++m_voiceLookupSerial;
    if (voiceId.isEmpty()) {
        m_voiceName->setText(tr("Enter a Voice ID to look up its name."));
        return;
    }
    if (apiKey.isEmpty()) {
        m_voiceName->setText(tr("Enter an API key to look up the voice name."));
        return;
    }

    m_voiceName->setText(tr("Looking up voice name…"));
    const QUrl url(QStringLiteral("https://api.elevenlabs.io/v1/voices/")
                   + QString::fromLatin1(QUrl::toPercentEncoding(voiceId)));
    QNetworkRequest request(url);
    request.setTransferTimeout(15000);
    request.setRawHeader("xi-api-key", apiKey.toUtf8());
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, voiceId, lookupSerial]() {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        const QString networkError = reply->errorString();
        reply->deleteLater();
        if (lookupSerial != m_voiceLookupSerial || voiceId != m_voiceId->text().trimmed())
            return;

        if (status >= 200 && status < 300 && document.isObject()) {
            const QString name = document.object().value(QStringLiteral("name")).toString().trimmed();
            m_voiceName->setText(name.isEmpty() ? tr("Voice name unavailable") : name);
        } else if (status == 401 || status == 403) {
            QString apiStatus;
            QString apiMessage;
            const QJsonValue detail = document.object().value(QStringLiteral("detail"));
            if (detail.isObject()) {
                const QJsonObject error = detail.toObject();
                apiStatus = error.value(QStringLiteral("status")).toString();
                apiMessage = error.value(QStringLiteral("message")).toString();
            } else if (detail.isString()) {
                apiMessage = detail.toString();
            }

            if (apiStatus == QLatin1String("invalid_api_key")) {
                m_voiceName->setText(tr("ElevenLabs rejected the API key. Check that it is correct and active."));
            } else if (apiStatus == QLatin1String("voice_not_found")) {
                m_voiceName->setText(tr("ElevenLabs could not find this Voice ID. Check the ID in My Voices."));
            } else if (!apiMessage.isEmpty()) {
                m_voiceName->setText(tr("ElevenLabs rejected the voice lookup (HTTP %1): %2")
                                         .arg(status).arg(apiMessage));
            } else {
                m_voiceName->setText(tr("ElevenLabs rejected the voice lookup (HTTP %1). Check the API key and enable Voices > Read access for this API key.")
                                         .arg(status));
            }
        } else if (status) {
            QString apiMessage;
            const QJsonValue detail = document.object().value(QStringLiteral("detail"));
            if (detail.isObject())
                apiMessage = detail.toObject().value(QStringLiteral("message")).toString();
            else if (detail.isString())
                apiMessage = detail.toString();
            m_voiceName->setText(apiMessage.isEmpty()
                                     ? tr("Could not retrieve voice name (HTTP %1). Check the Voice ID and API key.").arg(status)
                                     : tr("Could not retrieve voice name (HTTP %1): %2").arg(status).arg(apiMessage));
        } else {
            m_voiceName->setText(tr("Could not retrieve voice name: %1").arg(networkError));
        }
    });
}

void ElevenLabsSettingsDialog::refreshAzureVoices()
{
    const QString apiKey = m_apiKey->text().trimmed();
    const QString region = m_azureRegion->currentText().trimmed();
    if (apiKey.isEmpty() || region.isEmpty()) {
        m_testStatus->setText(tr("Enter the Azure subscription key and region to load voices."));
        return;
    }
    m_refreshAzureVoicesButton->setEnabled(false);
    m_testStatus->setText(tr("Loading Azure voices…"));
    QNetworkRequest request(QUrl(QStringLiteral("https://") + region
        + QStringLiteral(".tts.speech.microsoft.com/cognitiveservices/voices/list")));
    request.setTransferTimeout(20000);
    request.setRawHeader("Ocp-Apim-Subscription-Key", apiKey.toUtf8());
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, region]() {
        m_refreshAzureVoicesButton->setEnabled(true);
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const QString networkError = reply->errorString();
        reply->deleteLater();
        const QJsonDocument document = QJsonDocument::fromJson(body);
        if (status < 200 || status >= 300 || !document.isArray()) {
            QString details = QString::fromUtf8(body).trimmed();
            if (details.size() > 240) details.truncate(240);
            m_testStatus->setText(status
                ? tr("Azure voice list failed (HTTP %1). Check the key and region. %2").arg(status).arg(details)
                : tr("Could not connect to Azure Speech: %1").arg(networkError));
            return;
        }
        const QString previousVoice = m_azureVoice->currentText().trimmed();
        const QString localePrefix = QStringLiteral("es-");
        m_azureVoice->clear();
        int preferredIndex = -1;
        for (const QJsonValue &value : document.array()) {
            if (!value.isObject()) continue;
            const QJsonObject voice = value.toObject();
            const QString shortName = voice.value(QStringLiteral("ShortName")).toString();
            const QString locale = voice.value(QStringLiteral("Locale")).toString();
            if (shortName.isEmpty()) continue;
            const QString display = voice.value(QStringLiteral("DisplayName")).toString();
            m_azureVoice->addItem(QStringLiteral("%1 — %2 (%3)").arg(display, shortName, locale), shortName);
            const int index = m_azureVoice->count() - 1;
            m_azureVoice->setItemData(index, locale, Qt::UserRole + 1);
            if (shortName == previousVoice) preferredIndex = index;
            if (preferredIndex < 0 && locale.startsWith(localePrefix)) preferredIndex = index;
        }
        if (preferredIndex >= 0) m_azureVoice->setCurrentIndex(preferredIndex);
        else m_azureVoice->setEditText(previousVoice);
        ProviderDraft &draft = m_providerDrafts["azure"];
        draft.region = region;
        draft.voiceName = azureVoiceName();
        draft.voiceLocale = azureVoiceLocale();
        m_testStatus->setText(tr("Loaded %1 Azure voices for %2.").arg(m_azureVoice->count()).arg(region));
    });
}

bool ElevenLabsSettingsDialog::saveSettings()
{
    captureProviderDraft();
    const ProviderDraft eleven = m_providerDrafts.value("elevenlabs");
    const ProviderDraft azure = m_providerDrafts.value("azure");
    const QByteArray protectedElevenKey = protectApiKey(eleven.apiKey);
    const QByteArray protectedAzureKey = protectApiKey(azure.apiKey);
#ifdef Q_OS_WIN
    if ((!eleven.apiKey.isEmpty() && protectedElevenKey.isEmpty())
        || (!azure.apiKey.isEmpty() && protectedAzureKey.isEmpty())) {
        QMessageBox::warning(this, tr("AI voice settings"),
                             tr("Windows could not securely save the API key."));
        return false;
    }
#endif
    QSettings stored(QSettings::IniFormat, QSettings::UserScope, "Radit", "Radit");
    stored.beginGroup("ElevenLabs");
    if (eleven.apiKey.isEmpty())
        stored.remove("apiKey");
    else
        stored.setValue("apiKey", protectedElevenKey);
    stored.setValue("voiceId", eleven.voiceId);
    stored.setValue("model", eleven.model.isEmpty() ? QStringLiteral("eleven_multilingual_v2") : eleven.model);
    stored.endGroup();
    stored.beginGroup("AzureSpeech");
    if (azure.apiKey.isEmpty()) stored.remove("apiKey");
    else stored.setValue("apiKey", protectedAzureKey);
    stored.setValue("region", azure.region.isEmpty() ? QStringLiteral("westeurope") : azure.region);
    stored.setValue("voiceName", azure.voiceName);
    stored.setValue("voiceLocale", azure.voiceLocale.isEmpty() ? QStringLiteral("es-ES") : azure.voiceLocale);
    stored.endGroup();
    stored.beginGroup("AIVoices");
    stored.setValue("provider", selectedProvider());
    stored.endGroup();
    stored.sync();
    if (stored.status() != QSettings::NoError) {
        QMessageBox::warning(this, tr("AI voice settings"),
                             tr("Radit could not save the AI voice settings."));
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
