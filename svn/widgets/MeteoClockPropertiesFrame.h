#ifndef METEOCLOCKPROPERTIESFRAME_H
#define METEOCLOCKPROPERTIESFRAME_H

#include "widgets/AudioItemMeteoClockMaxi.h"
#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWindow>

class MeteoClockPropertiesFrame final : public Frame
{
public:
    explicit MeteoClockPropertiesFrame(AudioItemMeteoClockMaxi *item)
        : Frame(item), m_item(item)
    {
        setObjectName("MeteoClockPropertiesFrame");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowModality(Qt::WindowModal);
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle(tr("MeteoClock properties"));
        setWindowIcon(QIcon(":/icons/properties.svg"));
        resize(460, 175);

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
        closeButton->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
        closeButton->setFixedSize(15, 15);
        closeButton->SetIcon("Close-hover.svg");
        closeButton->setIconSize(QSize(15, 15));
        closeButton->setToolTip(tr("Close MeteoClock properties"));
        barLayout->addWidget(closeButton);
        connect(closeButton, &Button::clicked, this, &QWidget::close);
        root->addWidget(m_titleBar);

        auto *content = new QWidget(this);
        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(10, 8, 10, 8);
        layout->setSpacing(8);
        root->addWidget(content);

        auto *voicePackRow = new QHBoxLayout;
        voicePackRow->setSpacing(8);
        auto *voicePackLabel = new Label(content);
        voicePackLabel->setText(tr("Voice pack"));
        voicePackLabel->setFixedWidth(90);
        m_voicePackPath = new QLineEdit(item->filePath(), content);
        m_voicePackPath->setObjectName("MeteoVoicePackPath");
        m_voicePackPath->setReadOnly(true);
        m_voicePackPath->setToolTip(item->filePath());
        m_voicePackPath->setStyleSheet(
            "QLineEdit { background-color: #252b39; color: #80A4AE; "
            "border: 1px solid #535b6a; border-radius: 3px; "
            "padding: 2px 6px; selection-background-color: #4e4d7a; "
            "selection-color: white; } "
            "QLineEdit:focus { border-color: #80A4AE; }");
        auto *browse = new Button(content);
        browse->setText(tr("Browse..."));
        browse->setFixedHeight(24);
        voicePackRow->addWidget(voicePackLabel);
        voicePackRow->addWidget(m_voicePackPath, 1);
        voicePackRow->addWidget(browse);
        layout->addLayout(voicePackRow);

        auto *announceLabel = new Label(content);
        announceLabel->setText(tr("Announcement"));
        layout->addWidget(announceLabel);
        m_time = new QCheckBox(tr("Time"), content);
        m_time->setObjectName("AnnounceTime");
        m_time->setChecked(item->announcesTime());
        m_weather = new QCheckBox(tr("Weather"), content);
        m_weather->setObjectName("AnnounceWeather");
        m_weather->setChecked(item->announcesWeather());
        auto *options = new QHBoxLayout;
        options->setContentsMargins(4, 0, 0, 0);
        options->setSpacing(18);
        options->addWidget(m_time);
        options->addWidget(m_weather);
        options->addStretch();
        layout->addLayout(options);

        auto *buttons = new QHBoxLayout;
        buttons->setSpacing(8);
        buttons->addStretch();
        auto *cancel = new Button(content);
        cancel->setText(tr("Cancel"));
        auto *save = new Button(content);
        save->setText(tr("Save"));
        save->setObjectName("SaveMeteoClockProperties");
        cancel->setFixedSize(70, 24);
        save->setFixedSize(70, 24);
        buttons->addWidget(cancel);
        buttons->addWidget(save);
        layout->addLayout(buttons);

        const auto validate = [this, save]() {
            const bool hasVoicePack = !m_voicePackPath->text().isEmpty();
            save->setEnabled((m_time->isChecked() || m_weather->isChecked())
                             && (!hasVoicePack || QFileInfo(m_voicePackPath->text()).isFile()));
        };
        connect(m_time, &QCheckBox::toggled, this, [this, validate](bool checked) {
            if (!checked && !m_weather->isChecked()) {
                m_time->blockSignals(true);
                m_time->setChecked(true);
                m_time->blockSignals(false);
            }
            validate();
        });
        connect(m_weather, &QCheckBox::toggled, this, [this, validate](bool checked) {
            if (!checked && !m_time->isChecked()) {
                m_weather->blockSignals(true);
                m_weather->setChecked(true);
                m_weather->blockSignals(false);
            }
            validate();
        });
        connect(browse, &Button::clicked, this, [this]() {
            const QString path = QFileDialog::getOpenFileName(
                this, tr("MeteoClock voice pack"), m_voicePackPath->text(), tr("Voice packs (*.zip)"));
            if (!path.isEmpty()) {
                m_voicePackPath->setText(QFileInfo(path).absoluteFilePath());
                m_voicePackPath->setToolTip(m_voicePackPath->text());
            }
        });
        connect(cancel, &Button::clicked, this, &QWidget::close);
        connect(save, &Button::clicked, this, [this]() {
            if (!m_item) {
                close();
                return;
            }
            m_item->setAnnouncementOptions(m_time->isChecked(), m_weather->isChecked());
            const QString selectedPath = m_voicePackPath->text();
            if (!selectedPath.isEmpty() && selectedPath != m_item->filePath())
                m_item->setVoicePackPath(selectedPath);
            close();
        });
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
        connect(escape, &QShortcut::activated, this, &QWidget::close);
        validate();
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
    QPointer<AudioItemMeteoClockMaxi> m_item;
    Frame *m_titleBar = nullptr;
    QLineEdit *m_voicePackPath = nullptr;
    QCheckBox *m_time = nullptr;
    QCheckBox *m_weather = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};

#endif // METEOCLOCKPROPERTIESFRAME_H
