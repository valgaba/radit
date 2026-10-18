/* This file is part of Radit.
   Copyright 2026, Victor Algaba <victorengine@gmail.com> www.radit.org
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "widgets/NetPropertiesDialog.h"

#include "widgets/button.h"
#include "widgets/frame.h"
#include "widgets/label.h"
#include "core/MediaManager.h"

#include <QDialogButtonBox>
#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLocale>
#include <QMouseEvent>
#include <QPushButton>
#include <QShortcut>
#include <QTime>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <QWindow>

NetPropertiesDialog::NetPropertiesDialog(const QString &name, const QString &url,
                                         int connectionDurationSeconds, QWidget *parent,
                                         bool mixEnabled, double mixSeconds)
    : QDialog(parent)
{
    setObjectName("OnlineRadioDialog");
    setWindowTitle(tr("Online radio properties"));
    setWindowIcon(QIcon(":/icons/radit.ico"));
    setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
    setWindowModality(Qt::WindowModal);
    resize(520,connectionDurationSeconds>=0 ? 285 : 245);
    setStyleSheet(
        "QDialog#OnlineRadioDialog { background:#181e2c; border:1px solid #77859a; }"
        "QDialog#OnlineRadioDialog QLabel, QCheckBox { color:#8a8d96; }"
        "QDialog#OnlineRadioDialog QCheckBox { spacing:6px; }"
        "QDialog#OnlineRadioDialog QCheckBox::indicator { width:13px; height:13px;"
        " border:1px solid #5b6070; background:#202635; border-radius:2px; }"
        "QDialog#OnlineRadioDialog QCheckBox::indicator:checked {"
        " background:#80A4AE; border:1px solid #80A4AE; }"
        "QDialog#OnlineRadioDialog QLineEdit, QTimeEdit { color:#80A4AE; background:#282f40;"
        " border:1px solid #535b6a; border-radius:3px; padding:2px 6px;"
        " selection-background-color:#4e4d7a; selection-color:white; }"
        "QDialog#OnlineRadioDialog QLineEdit:focus, QTimeEdit:focus { border-color:#80A4AE; }"
        "QDialog#OnlineRadioDialog QTimeEdit::up-button, QTimeEdit::down-button { width:0; }"
        "QDialog#OnlineRadioDialog QPushButton { color:#8a8d96; background:#282f40;"
        " border:1px solid #5b6070; border-radius:2px; padding:2px 8px; }"
        "QDialog#OnlineRadioDialog QPushButton:hover { border-color:#80A4AE; background:#30384b; }"
        "QDialog#OnlineRadioDialog Frame#framebarra { background:#4e4d7a; border:none; }");

    auto *root=new QVBoxLayout(this);
    root->setContentsMargins(0,0,0,0);
    root->setSpacing(0);

    m_titleBar=new Frame(this);
    m_titleBar->setObjectName("framebarra");
    m_titleBar->setFixedHeight(25);
    m_titleBar->installEventFilter(this);
    auto *barLayout=new QHBoxLayout(m_titleBar);
    barLayout->setContentsMargins(0,0,0,0);
    barLayout->setSpacing(0);
    auto *title=new Label(m_titleBar);
    title->setObjectName("PanelTitle");
    title->setText(windowTitle());
    title->setAttribute(Qt::WA_TransparentForMouseEvents);
    barLayout->addWidget(title);
    barLayout->addStretch();
    auto *close=new Button(m_titleBar);
    close->setStyleSheet("QPushButton { border: none; background: transparent; padding: 0px; }");
    close->setFixedSize(15,15);
    close->SetIcon("Close-hover.svg");
    close->setIconSize(QSize(15,15));
    close->setToolTip(tr("Close online radio properties"));
    barLayout->addWidget(close);
    connect(close,&Button::clicked,this,&QDialog::reject);
    root->addWidget(m_titleBar);

    auto *content=new QWidget(this);
    auto *contentLayout=new QVBoxLayout(content);
    contentLayout->setContentsMargins(10,8,10,8);
    contentLayout->setSpacing(8);
    root->addWidget(content);

    auto *form=new QFormLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    m_name=new QLineEdit(name,content);
    m_url=new QLineEdit(url,content);
    m_url->setPlaceholderText("https://server.example/stream");
    m_url->setToolTip(tr("Direct audio stream URL, not the station website"));
    form->addRow(tr("Name"),m_name);
    form->addRow(tr("Stream URL"),m_url);
    if (connectionDurationSeconds>=0) {
        m_connectionDuration=new QTimeEdit(QTime(0,0).addSecs(connectionDurationSeconds),content);
        m_connectionDuration->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_connectionDuration->setDisplayFormat("HH:mm:ss");
        m_connectionDuration->setMinimumTime(QTime(0,0,1));
        m_connectionDuration->setMaximumTime(QTime(23,59,59));
        m_connectionDuration->setToolTip(tr("Stop playback after this connection time"));
        form->addRow(tr("Connection time"),m_connectionDuration);
    }
    m_mixEnabled = new QCheckBox(tr("Mix before the audio ends"),content);
    m_mixEnabled->setObjectName("MixEnabled");
    m_mixEnabled->setChecked(mixEnabled);
    form->addRow(m_mixEnabled);
    m_mixSeconds = new QLineEdit(QString::number(mixSeconds,'f',1),content);
    m_mixSeconds->setObjectName("MixSeconds");
    auto *mixValidator = new QDoubleValidator(0.1,30.0,1,m_mixSeconds);
    mixValidator->setLocale(QLocale::c());
    mixValidator->setNotation(QDoubleValidator::StandardNotation);
    m_mixSeconds->setValidator(mixValidator);
    form->addRow(tr("Mix duration (seconds)"),m_mixSeconds);
    connect(m_mixEnabled,&QCheckBox::toggled,m_mixSeconds,&QLineEdit::setEnabled);
    m_mixSeconds->setEnabled(mixEnabled);
    contentLayout->addLayout(form);

    auto *buttons=new QHBoxLayout;
    buttons->setSpacing(8);
    buttons->addStretch();
    auto *cancel=new Button(content);
    cancel->setText(tr("Cancel"));
    auto *save=new Button(content);
    save->setText(tr("Save"));
    save->setObjectName("SaveOnlineRadioProperties");
    cancel->setFixedSize(70,24);
    save->setFixedSize(70,24);
    buttons->addWidget(cancel);
    buttons->addWidget(save);
    contentLayout->addLayout(buttons);

    const auto validate=[this,save]() {
        save->setEnabled(!m_name->text().trimmed().isEmpty()
                         && MediaManager::isNetworkUrl(m_url->text())
                         && (!m_mixEnabled->isChecked() || m_mixSeconds->hasAcceptableInput()));
    };
    connect(m_name,&QLineEdit::textChanged,this,validate);
    connect(m_url,&QLineEdit::textChanged,this,validate);
    connect(cancel,&Button::clicked,this,&QDialog::reject);
    connect(save,&Button::clicked,this,&QDialog::accept);
    auto *escape=new QShortcut(QKeySequence(Qt::Key_Escape),this);
    connect(escape,&QShortcut::activated,this,&QDialog::reject);
    validate();
}

QString NetPropertiesDialog::stationName() const
{
    return m_name->text().trimmed();
}

QString NetPropertiesDialog::streamUrl() const
{
    return m_url->text().trimmed();
}

bool NetPropertiesDialog::hasConnectionDuration() const
{
    return m_connectionDuration!=nullptr;
}

int NetPropertiesDialog::connectionDurationSeconds() const
{
    return m_connectionDuration ? QTime(0,0).secsTo(m_connectionDuration->time()) : -1;
}

bool NetPropertiesDialog::mixEnabled() const
{
    return m_mixEnabled && m_mixEnabled->isChecked();
}

double NetPropertiesDialog::mixSeconds() const
{
    bool ok=false;
    const double seconds=m_mixSeconds ? m_mixSeconds->text().toDouble(&ok) : 3.0;
    return ok ? seconds : 3.0;
}

bool NetPropertiesDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched==m_titleBar) {
        if (event->type()==QEvent::MouseButtonPress) {
            auto *mouse=static_cast<QMouseEvent*>(event);
            if (mouse->button()==Qt::LeftButton) {
                if (windowHandle() && windowHandle()->startSystemMove())
                    return true;
                m_dragging=true;
                m_dragOffset=mouse->globalPosition().toPoint()-frameGeometry().topLeft();
                return true;
            }
        } else if (event->type()==QEvent::MouseMove && m_dragging) {
            auto *mouse=static_cast<QMouseEvent*>(event);
            if (mouse->buttons()&Qt::LeftButton)
                move(mouse->globalPosition().toPoint()-m_dragOffset);
            return true;
        } else if (event->type()==QEvent::MouseButtonRelease) {
            m_dragging=false;
        }
    }
    return QDialog::eventFilter(watched,event);
}
