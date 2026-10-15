#ifndef ELEVENLABSSETTINGSDIALOG_H
#define ELEVENLABSSETTINGSDIALOG_H

#include <QDialog>
#include <QByteArray>
#include <QFormLayout>
#include <QHash>
#include <QPoint>
#include <QString>

class QComboBox;
class QLineEdit;
class QLabel;
class QNetworkAccessManager;
class QTemporaryDir;
class MediaManager;
class Button;
class Frame;

class ElevenLabsSettingsDialog final : public QDialog
{
public:
    explicit ElevenLabsSettingsDialog(QWidget *parent = nullptr);
    ~ElevenLabsSettingsDialog() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    struct ProviderDraft {
        QString apiKey;
        QString voiceId;
        QString model;
        QString region;
        QString voiceName;
        QString voiceLocale;
    };
    QString selectedProvider() const;
    QString azureVoiceName() const;
    QString azureVoiceLocale() const;
    void captureProviderDraft();
    void applyProviderUi();
    bool saveSettings();
    void testVoice();
    void refreshAccountInfo();
    void refreshVoiceName();
    void refreshAzureVoices();
    void clearPreview();
    void playPreview(const QByteArray &audio);

    QComboBox *m_provider = nullptr;
    QLabel *m_apiKeyLabel = nullptr;
    QLineEdit *m_apiKey = nullptr;
    QLineEdit *m_voiceId = nullptr;
    QLabel *m_voiceName = nullptr;
    QComboBox *m_model = nullptr;
    QComboBox *m_azureRegion = nullptr;
    QComboBox *m_azureVoice = nullptr;
    Button *m_refreshAzureVoicesButton = nullptr;
    QFormLayout *m_form = nullptr;
    QWidget *m_elevenLabsFields = nullptr;
    QWidget *m_azureFields = nullptr;
    QWidget *m_accountPanel = nullptr;
    QLabel *m_testStatus = nullptr;
    QLabel *m_accountName = nullptr;
    QLabel *m_creditStatus = nullptr;
    QLabel *m_accountStatus = nullptr;
    Button *m_testButton = nullptr;
    Button *m_refreshAccountButton = nullptr;
    QNetworkAccessManager *m_network = nullptr;
    MediaManager *m_previewMedia = nullptr;
    QTemporaryDir *m_previewDirectory = nullptr;
    Frame *m_titleBar = nullptr;
    bool m_dragging = false;
    QString m_displayedProvider;
    QPoint m_dragOffset;
    quint64 m_voiceLookupSerial = 0;
    QHash<QString, ProviderDraft> m_providerDrafts;
};

#endif // ELEVENLABSSETTINGSDIALOG_H
