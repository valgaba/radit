#ifndef FOLDERPROPERTIESFRAME_H
#define FOLDERPROPERTIESFRAME_H

#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/Player.h"
#include "widgets/Planner.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QFormLayout>
#include <QElapsedTimer>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QSlider>
#include <QShortcut>
#include <QWindow>
#include <memory>

class FolderPropertiesFrame : public Frame
{
public:
    explicit FolderPropertiesFrame(AudioItemFolderMaxi *item) : Frame(item)
    {
        const bool orderedList = item->isOrderedList();
        setObjectName("FolderPropertiesFrame");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowModality(Qt::WindowModal);
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle(orderedList ? tr("Audio list properties") : tr("Audio folder properties"));
        setWindowIcon(QIcon(":/icons/properties.svg"));
        resize(540, 520);
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
        auto *closeButton = new Button(m_titleBar);
        closeButton->setObjectName("CloseFolderProperties");
        closeButton->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
        closeButton->setFixedSize(15, 15);
        closeButton->SetIcon("Close-hover.svg");
        closeButton->setIconSize(QSize(15, 15));
        closeButton->setToolTip(tr("Close audio folder properties"));
        closeButton->setAccessibleName(closeButton->toolTip());
        barLayout->addWidget(closeButton);
        connect(closeButton, &Button::clicked, this, &QWidget::close);
        root->addWidget(m_titleBar);
        auto *content = new QWidget(this);
        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(8);
        root->addWidget(content);
        auto *form = new QFormLayout;
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(8);
        auto *folderRow = new QHBoxLayout;
        auto *folder = new QLineEdit(QDir::toNativeSeparators(item->folderPath()), this);
        folder->setCursorPosition(0);
        folder->setObjectName("FolderPath");
        auto *browse = new Button(this);
        browse->setText(tr("Browse..."));
        browse->setFixedHeight(24);
        folderRow->addWidget(folder, 1);
        folderRow->addWidget(browse);
        form->addRow(orderedList ? tr("Radit list") : tr("Folder"), folderRow);
        auto *songsPerTurn = new QLineEdit(QString::number(item->songsPerTurn()), this);
        songsPerTurn->setObjectName("FolderSongsPerTurn");
        songsPerTurn->setValidator(new QIntValidator(1, 999, songsPerTurn));
        songsPerTurn->setToolTip(tr("Number of tracks to play from this folder before moving to the next item."));
        if (orderedList)
            songsPerTurn->setVisible(false);
        else
            form->addRow(tr("Songs per turn"), songsPerTurn);
        auto *enabled = new QCheckBox(tr("Mix before the audio ends"), this);
        enabled->setObjectName("MixEnabled");
        enabled->setChecked(item->mixEnabled());
        form->addRow(enabled);
        auto *seconds = new QLineEdit(QString::number(item->mixSeconds(), 'f', 1), this);
        seconds->setObjectName("MixSeconds");
        auto *validator = new QDoubleValidator(0.1, 30.0, 1, seconds);
        validator->setLocale(QLocale::c());
        validator->setNotation(QDoubleValidator::StandardNotation);
        seconds->setValidator(validator);
        form->addRow(tr("Mix duration (seconds)"), seconds);

        auto *sweeperEnabled = new QCheckBox(tr("Enable sweeper (random station ID)"), this);
        sweeperEnabled->setObjectName("FolderSweeperEnabled");
        sweeperEnabled->setChecked(item->sweeperEnabled());
        form->addRow(sweeperEnabled);
        auto *sweeperFolderRow = new QWidget(this);
        auto *sweeperFolderLayout = new QHBoxLayout(sweeperFolderRow);
        sweeperFolderLayout->setContentsMargins(0, 0, 0, 0);
        sweeperFolderLayout->setSpacing(6);
        auto *sweeperFolder = new QLineEdit(item->sweeperFolderPath(), sweeperFolderRow);
        sweeperFolder->setObjectName("FolderSweeperFolder");
        sweeperFolder->setPlaceholderText(tr("Folder containing sweeper audio clips"));
        sweeperFolder->setToolTip(tr("A random clip is mixed over the track at the selected positions."));
        auto *browseSweeper = new Button(sweeperFolderRow);
        browseSweeper->setText(tr("Browse..."));
        browseSweeper->setFixedSize(68, 24);
        sweeperFolderLayout->addWidget(sweeperFolder, 1);
        sweeperFolderLayout->addWidget(browseSweeper);
        form->addRow(tr("Sweeper folder"), sweeperFolderRow);
        auto *sweeperPlacement = new QComboBox(this);
        sweeperPlacement->setObjectName("Combo");
        sweeperPlacement->addItem(tr("Entry and exit"), 0);
        sweeperPlacement->addItem(tr("Entry"), 1);
        sweeperPlacement->addItem(tr("Exit"), 2);
        const int placementIndex = item->sweeperAtEntry()
            ? (item->sweeperAtExit() ? 0 : 1)
            : (item->sweeperAtExit() ? 2 : 0);
        sweeperPlacement->setCurrentIndex(placementIndex);
        form->addRow(tr("Sweeper placement"), sweeperPlacement);
        auto *announcementsEnabled = new QCheckBox(tr("Enable voice announcements"), this);
        announcementsEnabled->setObjectName("FolderAnnouncementsEnabled");
        announcementsEnabled->setChecked(item->announcementsEnabled());
        form->addRow(announcementsEnabled);
        connect(browseSweeper, &Button::clicked, this, [this, sweeperFolder]() {
            const QString path = QFileDialog::getExistingDirectory(
                this, tr("Select sweeper folder"), sweeperFolder->text());
            if (!path.isEmpty()) sweeperFolder->setText(QDir::toNativeSeparators(path));
        });

        auto *introDelay = new QLineEdit(QString::number(item->announcementDelaySeconds()), this);
        introDelay->setObjectName("FolderAnnouncementDelay");
        introDelay->setValidator(new QIntValidator(0, 30, introDelay));
        introDelay->setToolTip(tr("Delay after the track starts before the opening announcement or entry sweeper."));
        form->addRow(tr("Opening announcement / sweeper after (seconds)"), introDelay);

        auto *voiceVolumeRow = new QWidget(this);
        auto *voiceVolumeLayout = new QHBoxLayout(voiceVolumeRow);
        voiceVolumeLayout->setContentsMargins(0, 0, 0, 0);
        voiceVolumeLayout->setSpacing(8);
        auto *voiceVolume = new QSlider(Qt::Horizontal, voiceVolumeRow);
        voiceVolume->setObjectName("FolderAnnouncementVolume");
        voiceVolume->setRange(0, 200);
        voiceVolume->setSingleStep(5);
        voiceVolume->setPageStep(10);
        voiceVolume->setValue(item->announcementVolumePercent());
        auto *voiceVolumeValue = new Label(voiceVolumeRow);
        voiceVolumeValue->setObjectName("FolderAnnouncementVolumeValue");
        voiceVolumeValue->setFixedWidth(48);
        voiceVolumeValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        voiceVolumeValue->setText(QStringLiteral("%1 %").arg(voiceVolume->value()));
        voiceVolumeLayout->addWidget(voiceVolume, 1);
        voiceVolumeLayout->addWidget(voiceVolumeValue);
        form->addRow(tr("Voice volume"), voiceVolumeRow);
        connect(voiceVolume, &QSlider::valueChanged, voiceVolumeValue,
                [voiceVolumeValue](int value) {
            voiceVolumeValue->setText(QStringLiteral("%1 %").arg(value));
        });

        auto *musicVolumeRow = new QWidget(this);
        auto *musicVolumeLayout = new QHBoxLayout(musicVolumeRow);
        musicVolumeLayout->setContentsMargins(0, 0, 0, 0);
        musicVolumeLayout->setSpacing(8);
        auto *musicVolume = new QSlider(Qt::Horizontal, musicVolumeRow);
        musicVolume->setObjectName("FolderAnnouncementMusicVolume");
        musicVolume->setRange(0, 100);
        musicVolume->setSingleStep(1);
        musicVolume->setPageStep(5);
        musicVolume->setValue(item->announcementMusicVolumePercent());
        auto *musicVolumeValue = new Label(musicVolumeRow);
        musicVolumeValue->setObjectName("FolderAnnouncementMusicVolumeValue");
        musicVolumeValue->setFixedWidth(48);
        musicVolumeValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        musicVolumeValue->setText(QStringLiteral("%1 %").arg(musicVolume->value()));
        musicVolumeLayout->addWidget(musicVolume, 1);
        musicVolumeLayout->addWidget(musicVolumeValue);
        form->addRow(tr("Background music volume"), musicVolumeRow);
        musicVolume->setToolTip(tr("Music level while the voice is speaking."));
        connect(musicVolume, &QSlider::valueChanged, musicVolumeValue,
                [musicVolumeValue](int value) {
            musicVolumeValue->setText(QStringLiteral("%1 %").arg(value));
        });

        auto *recovery = new QLineEdit(QString::number(item->announcementRecoverySeconds(), 'f', 1), this);
        recovery->setObjectName("FolderAnnouncementRecovery");
        auto *recoveryValidator = new QDoubleValidator(0.1, 5.0, 1, recovery);
        recoveryValidator->setLocale(QLocale::c());
        recoveryValidator->setNotation(QDoubleValidator::StandardNotation);
        recovery->setValidator(recoveryValidator);
        recovery->setToolTip(tr("How quickly the music returns to normal volume after the voice."));
        form->addRow(tr("Music recovery time (seconds)"), recovery);

        auto *introText = new QLineEdit(item->introAnnouncement().isEmpty()
            ? tr("[excited] ¡Sigues en tu emisora favorita! Te dejamos con [artist] y su tema [title].")
            : item->introAnnouncement(), this);
        introText->setObjectName("FolderIntroAnnouncement");
        introText->setPlaceholderText(tr("Example: You are listening to [artist], with [title] from [album]"));
        introText->setToolTip(tr("Use [artist], [title] and [album] for the ID3 artist, track title and album."));
        form->addRow(tr("Opening announcement"), introText);

        auto *outroText = new QLineEdit(item->outroAnnouncement().isEmpty()
            ? QString()
            : item->outroAnnouncement(), this);
        outroText->setObjectName("FolderOutroAnnouncement");
        outroText->setPlaceholderText(tr("Text to announce 10 seconds before the track ends"));
        outroText->setToolTip(tr("Use [artist], [title] and [album] for the ID3 artist, track title and album."));
        form->addRow(tr("Closing announcement"), outroText);

        auto *variables = new Label(this);
        variables->setText(tr("Available fields: [artist], [title], [album]. Audio tags such as [excited] require Eleven v3. Closing announcement: 10 seconds before the end."));
        variables->setWordWrap(true);
        form->addRow(QString(), variables);
        layout->addLayout(form);
        auto *buttons = new QHBoxLayout;
        buttons->setSpacing(8);
        auto *testVoice = new Button(this);
        testVoice->setText(tr("Test cached voice + music"));
        testVoice->setObjectName("TestCachedFolderVoice");
        testVoice->setFixedSize(145, 24);
        auto *testSweeper = new Button(this);
        testSweeper->setText(tr("Test sweeper + music"));
        testSweeper->setObjectName("TestFolderSweeper");
        testSweeper->setFixedSize(145, 24);
        buttons->addStretch();
        auto *cancel = new Button(this); cancel->setText(tr("Cancel"));
        auto *save = new Button(this); save->setText(tr("Save")); save->setObjectName("SaveFolderProperties");
        cancel->setFixedSize(70, 24); save->setFixedSize(70, 24);
        buttons->addWidget(testVoice); buttons->addWidget(testSweeper);
        buttons->addWidget(cancel); buttons->addWidget(save);
        layout->addLayout(buttons);
        connect(browse, &Button::clicked, this, [this, folder, orderedList]() {
            const QString path = orderedList
                ? QFileDialog::getOpenFileName(this, tr("Select Radit list"), folder->text(),
                                               tr("Radit List (*.list);;All files (*)"))
                : QFileDialog::getExistingDirectory(this, tr("Select audio folder"), folder->text());
            if (!path.isEmpty()) folder->setText(QDir::toNativeSeparators(path));
        });
        const auto validate = [=]() {
            seconds->setEnabled(enabled->isChecked());
            const bool speak = announcementsEnabled->isChecked();
            const bool useSweeper = sweeperEnabled->isChecked();
            const bool useVoiceMix = speak || useSweeper;
            const bool entryCue = speak || (useSweeper && sweeperPlacement->currentData().toInt() != 2);
            sweeperFolder->setEnabled(useSweeper);
            browseSweeper->setEnabled(useSweeper);
            sweeperPlacement->setEnabled(useSweeper);
            introDelay->setEnabled(entryCue);
            voiceVolume->setEnabled(useVoiceMix);
            musicVolume->setEnabled(useVoiceMix);
            recovery->setEnabled(useVoiceMix);
            introText->setEnabled(speak);
            outroText->setEnabled(speak);
            testVoice->setEnabled(speak);
            testSweeper->setEnabled(useSweeper && QFileInfo(sweeperFolder->text()).isDir());
            const QFileInfo sourceInfo(folder->text());
            const bool sourceValid = orderedList ? (sourceInfo.isFile()
                    && sourceInfo.suffix().compare("list", Qt::CaseInsensitive) == 0)
                : sourceInfo.isDir();
            save->setEnabled(sourceValid
                && (orderedList || songsPerTurn->hasAcceptableInput())
                && (!enabled->isChecked() || seconds->hasAcceptableInput())
                && (!useSweeper || QFileInfo(sweeperFolder->text()).isDir())
                && (!entryCue || introDelay->hasAcceptableInput())
                && (!useVoiceMix || recovery->hasAcceptableInput()));
        };
        connect(enabled, &QCheckBox::toggled, this, validate);
        connect(announcementsEnabled, &QCheckBox::toggled, this, validate);
        connect(sweeperEnabled, &QCheckBox::toggled, this, validate);
        connect(sweeperPlacement, &QComboBox::currentIndexChanged, this, validate);
        connect(sweeperFolder, &QLineEdit::textChanged, this, validate);
        connect(seconds, &QLineEdit::textChanged, this, validate);
        connect(songsPerTurn, &QLineEdit::textChanged, this, validate);
        connect(introDelay, &QLineEdit::textChanged, this, validate);
        connect(recovery, &QLineEdit::textChanged, this, validate);
        connect(folder, &QLineEdit::textChanged, this, validate);
        validate();
        connect(cancel, &Button::clicked, this, &QWidget::close);
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
        connect(escape, &QShortcut::activated, this, &QWidget::close);
        connect(save, &Button::clicked, item, [=]() {
            const QString path = QDir::cleanPath(QFileInfo(folder->text()).absoluteFilePath());
            if (path != item->folderPath()) {
                if (item->isPlaying()) {
                    QWidget *owner = item->parentWidget();
                    while (owner && !qobject_cast<Player*>(owner)) owner = owner->parentWidget();
                    if (auto *player = qobject_cast<Player*>(owner)) player->stopMain();
                }
                if (orderedList) {
                    QString error;
                    if (!item->setOrderedListPath(path, &error)) {
                        QMessageBox::warning(this, tr("Audio list"), error);
                        return;
                    }
                } else {
                    item->setFolderPath(path);
                }
            }
            item->setMixSettings(enabled->isChecked(), seconds->text().toDouble());
            if (!orderedList)
                item->setSongsPerTurn(songsPerTurn->text().toInt());
            item->setAnnouncementsEnabled(announcementsEnabled->isChecked());
            const int placement = sweeperPlacement->currentData().toInt();
            item->setSweeper(sweeperEnabled->isChecked(), sweeperFolder->text(),
                             placement != 2, placement != 1);
            item->setAnnouncementVolumePercent(voiceVolume->value());
            item->setAnnouncementMusicVolumePercent(musicVolume->value());
            item->setAnnouncementRecoverySeconds(recovery->text().toDouble());
            item->setAnnouncementTexts(introText->text(), outroText->text(),
                                       introDelay->text().toInt());
            close();
        });
        const auto previewClipWithMusic = [=](const QString &clipPath, const QString &titleText) {
            QString musicPath = item->playbackPath();
            if (musicPath.isEmpty() || !QFileInfo::exists(musicPath)) {
                QStringList availableTracks = item->isOrderedList()
                    ? item->orderedTrackPaths() : item->indexedTrackPaths();
                if (availableTracks.isEmpty() && !item->isOrderedList()) {
                    const QFileInfoList entries = QDir(item->folderPath()).entryInfoList(
                        MediaManager::supportedAudioNameFilters(),
                        QDir::Files | QDir::Readable, QDir::Name);
                    for (const QFileInfo &entry : entries)
                        availableTracks.append(entry.absoluteFilePath());
                }
                if (!availableTracks.isEmpty())
                    musicPath = availableTracks.at(
                        QRandomGenerator::global()->bounded(availableTracks.size()));
            }
            if (clipPath.isEmpty() || !QFileInfo::exists(clipPath)
                || musicPath.isEmpty() || !QFileInfo::exists(musicPath)) {
                QMessageBox::information(this, titleText,
                    tr("Radit could not find a music track or the selected audio clip for this test."));
                return;
            }
            Player *player = nullptr;
            Planner *planner = nullptr;
            QWidget *owner = item;
            while (owner && !player && !planner) {
                player = qobject_cast<Player*>(owner);
                planner = qobject_cast<Planner*>(owner);
                if (!player && !planner)
                    owner = owner->parentWidget();
            }
            const int device = player ? player->devicePlay()
                : planner ? planner->devicePlay() : 0;
            if (player && player->getCurrentItem() == item
                && player->previewFolderAnnouncement(clipPath, voiceVolume->value(),
                    musicVolume->value(), recovery->text().toDouble()))
                return;
            auto *musicPreview = new MediaManager(this);
            auto *clipPreview = new MediaManager(this);
            if (!musicPreview->setDevice(device) || !clipPreview->setDevice(device)
                || !musicPreview->loadFile(musicPath) || !clipPreview->loadFile(clipPath)) {
                QMessageBox::warning(this, titleText, tr("The selected audio clip or music track could not be opened."));
                musicPreview->deleteLater();
                clipPreview->deleteLater();
                return;
            }
            const float masterGain = player ? player->volume()
                : planner ? planner->volume() : 1.0f;
            const float normalMusicGain = masterGain;
            const float initialMusicGain = normalMusicGain * musicVolume->value() / 100.0f;
            const int recoveryMs = qRound(recovery->text().toDouble() * 1000.0);
            musicPreview->setVolume(initialMusicGain);
            clipPreview->setVolume(masterGain * voiceVolume->value() / 100.0f);
            auto *recoveryTimer = new QTimer(this);
            recoveryTimer->setInterval(25);
            auto recoveryClock = std::make_shared<QElapsedTimer>();
            connect(clipPreview, &MediaManager::playbackFinished, this,
                    [=]() {
                if (recoveryMs <= 100) {
                    musicPreview->stop();
                    clipPreview->stop();
                    musicPreview->deleteLater();
                    clipPreview->deleteLater();
                    recoveryTimer->deleteLater();
                    return;
                }
                recoveryClock->restart();
                recoveryTimer->start();
            });
            connect(recoveryTimer, &QTimer::timeout, this, [=]() {
                const float progress = qBound(0.0f,
                    float(recoveryClock->elapsed()) / float(recoveryMs), 1.0f);
                musicPreview->setVolume(initialMusicGain
                    + (normalMusicGain - initialMusicGain) * progress);
                if (progress >= 1.0f) {
                    recoveryTimer->stop();
                    musicPreview->stop();
                    clipPreview->stop();
                    musicPreview->deleteLater();
                    clipPreview->deleteLater();
                    recoveryTimer->deleteLater();
                }
            });
            connect(clipPreview, &MediaManager::playbackError, this, [=](const QString &) {
                recoveryTimer->stop();
                musicPreview->stop();
                clipPreview->stop();
                musicPreview->deleteLater();
                clipPreview->deleteLater();
                recoveryTimer->deleteLater();
            });
            musicPreview->play();
            clipPreview->play();
        };
        connect(testVoice, &Button::clicked, this, [=]() {
            if (introText->text().trimmed() != item->introAnnouncement()) {
                QMessageBox::information(this, tr("Test cached voice"),
                    tr("Save this text and play the folder once to generate its voice. The test button only replays cached audio and never contacts the selected AI voice provider."));
                return;
            }
            previewClipWithMusic(item->introAnnouncementPath(), tr("Test cached voice"));
        });
        connect(testSweeper, &Button::clicked, this, [=]() {
            const QFileInfoList sweepers = QDir(sweeperFolder->text()).entryInfoList(
                MediaManager::supportedAudioNameFilters(), QDir::Files | QDir::Readable,
                QDir::Name);
            if (sweepers.isEmpty()) {
                QMessageBox::information(this, tr("Test sweeper"),
                    tr("The selected sweeper folder does not contain any supported audio files."));
                return;
            }
            const int index = QRandomGenerator::global()->bounded(sweepers.size());
            previewClipWithMusic(sweepers.at(index).absoluteFilePath(), tr("Test sweeper"));
        });
    }
protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_titleBar) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *mouse = static_cast<QMouseEvent*>(event);
                if (mouse->button() == Qt::LeftButton) {
                    if (windowHandle() && windowHandle()->startSystemMove()) return true;
                    m_dragging = true;
                    m_dragOffset = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
                    return true;
                }
            } else if (event->type() == QEvent::MouseMove && m_dragging) {
                auto *mouse = static_cast<QMouseEvent*>(event);
                if (mouse->buttons() & Qt::LeftButton) move(mouse->globalPosition().toPoint() - m_dragOffset);
                return true;
            } else if (event->type() == QEvent::MouseButtonRelease) {
                m_dragging = false;
            }
        }
        return Frame::eventFilter(watched, event);
    }
private:
    Frame *m_titleBar = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};
#endif
