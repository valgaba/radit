#ifndef ELEVENLABSSETTINGSDIALOG_H
#define ELEVENLABSSETTINGSDIALOG_H

#include <QDialog>
#include <QPoint>

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
    bool saveSettings();
    void testVoice();
    void refreshAccountInfo();
    void clearPreview();

    QLineEdit *m_apiKey = nullptr;
    QLineEdit *m_voiceId = nullptr;
    QComboBox *m_model = nullptr;
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
    QPoint m_dragOffset;
};

#endif // ELEVENLABSSETTINGSDIALOG_H
