#ifndef QUITDIALOG_H
#define QUITDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QPushButton>

class QuitDialog : public QDialog
{
public:
    enum Choice { Cancel = QDialog::Rejected, Quit = QDialog::Accepted, SaveAndQuit = 2 };
    explicit QuitDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setObjectName("QuitDialog");
        setWindowTitle(tr("Quit!"));
        setWindowIcon(QIcon(":/icons/RaditLogo.svg"));
        setFixedSize(460, 150);
        auto *outer = new QHBoxLayout(this);
        outer->setContentsMargins(14, 14, 14, 14);
        outer->setSpacing(14);
        auto *logo = new QLabel(this);
        logo->setObjectName("QuitLogo");
        logo->setPixmap(QIcon(":/icons/RaditLogo.svg").pixmap(76, 76));
        logo->setFixedSize(76, 76);
        outer->addWidget(logo, 0, Qt::AlignTop);
        auto *content = new QVBoxLayout;
        content->setSpacing(10);
        auto *title = new QLabel(tr("Quit!"), this);
        title->setObjectName("QuitTitle");
        content->addWidget(title);
        auto *message = new QLabel(tr("Do you really want to quit?\nUnsaved lists and player layouts will be lost."), this);
        message->setWordWrap(true);
        content->addWidget(message);
        content->addStretch();
        auto *buttons = new QHBoxLayout;
        buttons->setSpacing(8);
        auto *quit = new QPushButton(tr("Quit"), this);
        quit->setObjectName("QuitWithoutSaving");
        quit->setFixedSize(70, 24);
        quit->setAutoDefault(false);
        buttons->addWidget(quit);
        buttons->addStretch();
        auto *cancel = new QPushButton(tr("Cancel"), this);
        cancel->setObjectName("QuitCancel");
        cancel->setFixedSize(70, 24);
        cancel->setDefault(true);
        buttons->addWidget(cancel);
        auto *save = new QPushButton(tr("Save & Quit"), this);
        save->setObjectName("SaveAndQuit");
        save->setFixedSize(94, 24);
        save->setAutoDefault(false);
        cancel->setFocus();
        buttons->addWidget(save);
        content->addLayout(buttons);
        outer->addLayout(content, 1);
        connect(quit, &QPushButton::clicked, this, [this]() { done(Quit); });
        connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
        connect(save, &QPushButton::clicked, this, [this]() { done(SaveAndQuit); });
    }
};

#endif
