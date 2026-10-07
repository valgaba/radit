#ifndef FOLDERPROPERTIESFRAME_H
#define FOLDERPROPERTIESFRAME_H

#include "widgets/AudioItemFolderMaxi.h"
#include "widgets/Player.h"
#include <QCheckBox>
#include <QDoubleValidator>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QShortcut>
#include <QWindow>

class FolderPropertiesFrame : public Frame
{
public:
    explicit FolderPropertiesFrame(AudioItemFolderMaxi *item) : Frame(item)
    {
        setObjectName("FolderPropertiesFrame");
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setWindowModality(Qt::WindowModal);
        setAttribute(Qt::WA_DeleteOnClose);
        setWindowTitle(tr("Audio folder properties"));
        setWindowIcon(QIcon(":/icons/properties.svg"));
        resize(520, 175);
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
        form->addRow(tr("Folder"), folderRow);
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
        layout->addLayout(form);
        auto *buttons = new QHBoxLayout;
        buttons->setSpacing(8);
        buttons->addStretch();
        auto *cancel = new Button(this); cancel->setText(tr("Cancel"));
        auto *save = new Button(this); save->setText(tr("Save")); save->setObjectName("SaveFolderProperties");
        cancel->setFixedSize(70, 24); save->setFixedSize(70, 24);
        buttons->addWidget(cancel); buttons->addWidget(save);
        layout->addLayout(buttons);
        connect(browse, &Button::clicked, this, [this, folder]() {
            const QString path = QFileDialog::getExistingDirectory(this, tr("Select audio folder"), folder->text());
            if (!path.isEmpty()) folder->setText(QDir::toNativeSeparators(path));
        });
        const auto validate = [=]() {
            seconds->setEnabled(enabled->isChecked());
            save->setEnabled(QFileInfo(folder->text()).isDir() && (!enabled->isChecked() || seconds->hasAcceptableInput()));
        };
        connect(enabled, &QCheckBox::toggled, this, validate);
        connect(seconds, &QLineEdit::textChanged, this, validate);
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
                item->setFolderPath(path);
            }
            item->setMixSettings(enabled->isChecked(), seconds->text().toDouble());
            close();
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
