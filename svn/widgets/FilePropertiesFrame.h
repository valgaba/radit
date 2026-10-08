#ifndef FILEPROPERTIESFRAME_H
#define FILEPROPERTIESFRAME_H

#include "core/MediaManager.h"
#include "widgets/AudioItemFileMaxi.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include <QFileInfo>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QTime>
#include <QVBoxLayout>
#include <QWindow>

class FilePropertiesFrame : public Frame
{
public:
    explicit FilePropertiesFrame(AudioItemFileMaxi *item) : Frame(item)
    {
        setObjectName("FilePropertiesFrame");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowModality(Qt::WindowModal);
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle(tr("Audio file properties"));
        setWindowIcon(QIcon(":/icons/properties.svg"));
        resize(600, 420);

        const AudioFileMetadata data = MediaManager::readAudioFileMetadata(item->filePath());
        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);
        m_titleBar = new Frame(this);
        m_titleBar->setObjectName("framebarra");
        m_titleBar->setFixedHeight(29);
        m_titleBar->installEventFilter(this);
        auto *barLayout = new QHBoxLayout(m_titleBar);
        barLayout->setContentsMargins(0, 0, 0, 0);
        barLayout->setSpacing(0);
        auto *title = new QLabel(m_titleBar);
        title->setObjectName("PanelTitle");
        title->setText(windowTitle());
        title->setAttribute(Qt::WA_TransparentForMouseEvents);
        barLayout->addWidget(title);
        barLayout->addStretch();
        auto *closeButton = new Button(m_titleBar);
        closeButton->setObjectName("CloseFileProperties");
        closeButton->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
        closeButton->setFixedSize(15, 15);
        closeButton->SetIcon("Close-hover.svg");
        closeButton->setIconSize(QSize(15, 15));
        closeButton->setToolTip(tr("Close audio file properties"));
        barLayout->addWidget(closeButton);
        connect(closeButton, &Button::clicked, this, &QWidget::close);
        root->addWidget(m_titleBar);

        auto *content = new QWidget(this);
        auto *layout = new QVBoxLayout(content);
        layout->setContentsMargins(12, 10, 12, 10);
        layout->setSpacing(7);
        root->addWidget(content);
        addRow(layout, tr("Title"), data.title.isEmpty() ? QFileInfo(item->filePath()).completeBaseName() : data.title);
        addRow(layout, tr("Artist"), data.artist);
        addRow(layout, tr("Album"), data.album);
        addRow(layout, tr("Year"), data.year);
        addRow(layout, tr("Comment"), data.comment);
        addRow(layout, tr("Genre"), data.genre);
        addRow(layout, tr("Duration"), data.duration >= 0.0
                    ? QTime(0, 0).addSecs(qRound(data.duration)).toString("hh:mm:ss") : tr("Unknown"));
        addRow(layout, tr("Audio format"), data.format.isEmpty() ? tr("Unknown") : data.format);
        addRow(layout, tr("File size"), data.sizeBytes > 0 ? QLocale().formattedDataSize(data.sizeBytes) : tr("Unknown"));
        auto *pathLabel = new QLabel(content);
        pathLabel->setText(tr("Path"));
        pathLabel->setObjectName("FilePropertyName");
        pathLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        auto *pathValue = new QLabel(content);
        pathValue->setText(QDir::toNativeSeparators(QFileInfo(item->filePath()).absoluteFilePath()));
        pathValue->setObjectName("FilePropertyPath");
        pathValue->setWordWrap(true);
        pathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
        pathValue->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        auto *pathRow = new QHBoxLayout;
        pathRow->setSpacing(12);
        pathLabel->setFixedWidth(82);
        pathRow->addWidget(pathLabel, 0, Qt::AlignTop);
        pathRow->addWidget(pathValue, 1);
        layout->addLayout(pathRow);
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_titleBar) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *mouse = static_cast<QMouseEvent *>(event);
                if (mouse->button() == Qt::LeftButton) {
                    if (windowHandle() && windowHandle()->startSystemMove()) return true;
                    m_dragging = true;
                    m_dragOffset = mouse->globalPosition().toPoint() - frameGeometry().topLeft();
                    return true;
                }
            } else if (event->type() == QEvent::MouseMove && m_dragging) {
                auto *mouse = static_cast<QMouseEvent *>(event);
                if (mouse->buttons() & Qt::LeftButton) move(mouse->globalPosition().toPoint() - m_dragOffset);
                return true;
            } else if (event->type() == QEvent::MouseButtonRelease) {
                m_dragging = false;
            }
        }
        return Frame::eventFilter(watched, event);
    }

private:
    static void addRow(QVBoxLayout *layout, const QString &name, const QString &value)
    {
        auto *row = new QHBoxLayout;
        row->setSpacing(12);
        auto *key = new QLabel(layout->parentWidget());
        key->setText(name);
        key->setObjectName("FilePropertyName");
        key->setFixedWidth(82);
        key->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        auto *text = new QLabel(layout->parentWidget());
        text->setText(value.isEmpty() ? QStringLiteral("—") : value);
        text->setObjectName("FilePropertyValue");
        text->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        text->setTextInteractionFlags(Qt::TextSelectableByMouse);
        text->setMinimumHeight(27);
        row->addWidget(key);
        row->addWidget(text, 1);
        layout->addLayout(row);
    }

    Frame *m_titleBar = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};

#endif // FILEPROPERTIESFRAME_H
