/* This file is part of Radit.
   Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

   Radit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Iradit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QLabel>
#include <QSlider>
#include <QSignalBlocker>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QSpinBox>
#include <cmath>

#include "widgets/frameoptionsplayer.h"
#include "widgets/label.h"
#include "widgets/Player.h"
#include "widgets/Planner.h"
#include "core/MediaManager.h"




FrameOptionsPlayer::FrameOptionsPlayer(QWidget *parent):Frame(parent){

    const bool showCueDevice=qobject_cast<Planner *>(parent)==nullptr;
    m_isPlannerOptions=!showCueDevice;
    this->setObjectName("FrameOptionsPlayer"); //para qss
    this->setWindowFlags((m_isPlannerOptions ? Qt::Tool : Qt::Popup)
                         | Qt::FramelessWindowHint);
    if (m_isPlannerOptions)
        QApplication::instance()->installEventFilter(this);
    this->setFixedSize(showCueDevice ? 350 : 430, showCueDevice ? 250 : 400);

    layout = new QVBoxLayout(this);  // layout general
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);


    frametop = new Frame(this);
    framecenter = new Frame(this);
    framedown = new Frame(this);

    framecenter->setObjectName("framecenter");
    frametop->setObjectName("frametop");
    framedown->setObjectName("framedown");


    layouttop = new QHBoxLayout(frametop);
    layoutcenter = new QFormLayout(framecenter);
    layoutdown = new QHBoxLayout(framedown);

    layoutdown->setContentsMargins(1, 1, 1, 1);
    layoutdown->setSpacing(10); // espacios entre  item dentro del contenedor

    layouttop->setContentsMargins(0, 0, 0, 0);
    layouttop->setSpacing(0); // espacios entre  item dentro del contenedor



    layout->addWidget(frametop);
    layout->addWidget(framecenter);
    layout->addWidget(framedown);

    layout->setStretch(0, 1); // arriba
    layout->setStretch(1, 4); // centro
    layout->setStretch(2, 1); // abajo

    frametop->setFixedHeight(23);

    //parte alta********************************************

    frametop->setStyleSheet("background-color: #4e4d7a; border: none;");


     optionsTitle = new Label(this);
     optionsTitle->setMaximumHeight(QWIDGETSIZE_MAX);
     optionsTitle->setWordWrap(false);
     QFont font;
          font.setPointSize(10);
          font.setBold(true);
          optionsTitle->setFont(font);

     optionsTitle->setText(qobject_cast<Planner *>(parent)
                               ? tr("Planner options")
                               : tr("Player options"));
     layouttop->addWidget(optionsTitle);

//parte media **************************************

    Label *labelplay= new Label(this);
    labelplay->setMaximumHeight(QWIDGETSIZE_MAX);
    labelplay->setWordWrap(false);
    labelplay->setFont(font);
    labelplay->setText("Play device");

    Label *labelcue= new Label(this);
    labelcue->setMaximumHeight(QWIDGETSIZE_MAX);
    labelcue->setWordWrap(false);
    labelcue->setFont(font);
    labelcue->setText("Cue device");

     comboplay = new QComboBox(this);
     comboplay->setObjectName("Combo"); //para qss

     combocue = new QComboBox(this);
     combocue->setObjectName("Combo"); //para qss

    layoutcenter->addRow(labelplay, comboplay);
    if (showCueDevice)
        layoutcenter->addRow(labelcue, combocue);
    else {
        labelcue->hide();
        combocue->hide();
    }

    auto *volumeRow = new QWidget(framecenter);
    auto *volumeLayout = new QHBoxLayout(volumeRow);
    volumeLayout->setContentsMargins(0, 0, 0, 0);
    volumeLayout->setSpacing(6);
    volumeSlider = new QSlider(Qt::Horizontal, volumeRow);
    volumeSlider->setObjectName("PlayerVolumeSlider");
    volumeSlider->setRange(0, 100);
    volumeSlider->setSingleStep(1);
    volumeSlider->setPageStep(10);
    volumeSlider->setValue(100);
    volumeSlider->setFixedHeight(24);
    volumeSlider->setToolTip(tr("Playback volume"));
    volumeSlider->setAccessibleName(tr("Playback volume"));
    volumeValue = new QLabel("100 %", volumeRow);
    volumeValue->setObjectName("PlayerVolumeValue");
    volumeValue->setFixedWidth(45);
    volumeValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    volumeLayout->addWidget(volumeSlider, 1);
    volumeLayout->addWidget(volumeValue);
    auto *volumeLabel = new Label(framecenter);
    volumeLabel->setText(tr("Volume"));
    volumeLabel->setFont(font);
    volumeLabel->setBuddy(volumeSlider);
    layoutcenter->addRow(volumeLabel, volumeRow);
    if (!showCueDevice) {
        auto *sectionSpacer=new QWidget(framecenter);
        sectionSpacer->setFixedHeight(9);
        layoutcenter->addRow(sectionSpacer);

        auto *sweeperTitle=new Label(framecenter);
        sweeperTitle->setText(tr("Scheduler sweepers"));
        sweeperTitle->setFont(font);
        layoutcenter->addRow(sweeperTitle);

        auto *planner=qobject_cast<Planner *>(parent);
        auto *folderRow=new QWidget(framecenter);
        auto *folderLayout=new QHBoxLayout(folderRow);
        folderLayout->setContentsMargins(0,0,0,0);
        folderLayout->setSpacing(5);
        schedulerSweeperFolder=new QLineEdit(
            planner ? planner->schedulerSweeperFolder() : QString(),folderRow);
        schedulerSweeperFolder->setObjectName("PlannerSchedulerSweeperFolder");
        schedulerSweeperFolder->setStyleSheet(
            "QLineEdit { color:#80A4AE; background:#282f40; border:1px solid #535b6a;"
            " min-height:20px; padding:2px 5px; }"
            "QLineEdit:focus { border-color:#80A4AE; }");
        schedulerSweeperFolder->setPlaceholderText(tr("Select a folder"));
        auto *browse=new Button(folderRow);
        browse->setText(tr("Browse…"));
        browse->setFixedWidth(74);
        folderLayout->addWidget(schedulerSweeperFolder,1);
        folderLayout->addWidget(browse);
        auto *folderLabel=new Label(framecenter);
        folderLabel->setText(tr("Common sweeper folder"));
        folderLabel->setFont(font);
        layoutcenter->addRow(folderLabel,folderRow);

        schedulerSweeperMaximumWindow=new QSpinBox(framecenter);
        schedulerSweeperMaximumWindow->setRange(1,600);
        schedulerSweeperMaximumWindow->setButtonSymbols(QAbstractSpinBox::NoButtons);
        schedulerSweeperMaximumWindow->setSuffix(tr(" s"));
        schedulerSweeperMaximumWindow->setStyleSheet(
            "QSpinBox { color:#80A4AE; background:#282f40; border:1px solid #535b6a;"
            " min-height:20px; padding:2px 5px; }"
            "QSpinBox:focus { border-color:#80A4AE; }");
        schedulerSweeperMaximumWindow->setValue(
            planner ? planner->schedulerSweeperMaxWindowSeconds() : 60);
        auto *windowLabel=new Label(framecenter);
        windowLabel->setText(tr("Maximum time to fill"));
        windowLabel->setFont(font);
        layoutcenter->addRow(windowLabel,schedulerSweeperMaximumWindow);

        schedulerSweeperAllowedOverrun=new QSpinBox(framecenter);
        schedulerSweeperAllowedOverrun->setRange(0,30);
        schedulerSweeperAllowedOverrun->setButtonSymbols(QAbstractSpinBox::NoButtons);
        schedulerSweeperAllowedOverrun->setSuffix(tr(" s"));
        schedulerSweeperAllowedOverrun->setStyleSheet(
            "QSpinBox { color:#80A4AE; background:#282f40; border:1px solid #535b6a;"
            " min-height:20px; padding:2px 5px; }"
            "QSpinBox:focus { border-color:#80A4AE; }");
        schedulerSweeperAllowedOverrun->setValue(
            planner ? planner->schedulerSweeperOverrunSeconds() : 3);
        schedulerSweeperAllowedOverrun->setToolTip(
            tr("Allow the event to start this many seconds late."));
        auto *overrunLabel=new Label(framecenter);
        overrunLabel->setText(tr("Allowed event delay"));
        overrunLabel->setFont(font);
        layoutcenter->addRow(overrunLabel,schedulerSweeperAllowedOverrun);

        connect(browse,&QPushButton::clicked,this,[this]() {
            const QString currentFolder=schedulerSweeperFolder->text();
            QWidget *dialogParent=parentWidget() ? parentWidget()->window() : nullptr;
            m_folderDialogOpen=true;
            const QString selected=QFileDialog::getExistingDirectory(
                dialogParent,tr("Select common sweeper folder"),currentFolder);
            m_folderDialogOpen=false;
            if (!selected.isEmpty())
                schedulerSweeperFolder->setText(QDir::toNativeSeparators(selected));
        });
    }
    connect(volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        auto *player = qobject_cast<Player *>(parentWidget());
        auto *planner = qobject_cast<Planner *>(parentWidget());
        if (!player && !planner)
            return;
        const bool accepted = player
            ? player->setVolume(value / 100.0f)
            : (planner && planner->setVolume(value / 100.0f));
        if (accepted) {
            volumeValue->setText(QString::number(value) + " %");
        } else {
            const QSignalBlocker blocker(volumeSlider);
            const int current = qRound((player ? player->volume() : planner->volume()) * 100);
            volumeSlider->setValue(current);
            volumeValue->setText(QString::number(current) + " %");
        }
    });




    //parte baja********************************************

    btncancel = new Button(this);
    btncancel->setText("Cancel");

    btncancel->setStyleSheet(
        "QPushButton {"
        "    padding: 4px 12px;"
        "}"
    );


    btnacept = new Button(this);
    btnacept->setText("Acept");

    btnacept->setStyleSheet(
        "QPushButton {"
        "    padding: 4px 12px;"
        "}"
    );

    layoutdown->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum)); //espaciador


    layoutdown->addWidget(btnacept);
    layoutdown->addWidget(btncancel);



    // Cancelar -> ocultar el panel
    connect(btncancel, &QPushButton::clicked,
            this, &FrameOptionsPlayer::hide);



    // aceptar ->
    connect(btnacept, &QPushButton::clicked, this, [this]() {

           int selectedPlay = comboplay->currentData().toInt();
           int selectedCue = combocue->currentData().toInt();

           // cast  clase padre

          Player *player = qobject_cast<Player*>(this->parent());
          Planner *planner = qobject_cast<Planner*>(this->parent());

           if (player) {
               player->setDevicePlay(selectedPlay);
               player->setDeviceCue(selectedCue);
               m_volumeCommitted = true;
               emit player->configurationChanged();
           } else if (planner) {
               const QString folder=schedulerSweeperFolder->text().trimmed();
               if (!folder.isEmpty() && !QFileInfo(folder).isDir()) {
                   QMessageBox::warning(this,tr("Scheduler sweepers"),
                                        tr("The common sweeper folder does not exist."));
                   return;
               }
               if (!planner->setSchedulerSweeperSettings(folder,
                       schedulerSweeperMaximumWindow->value(),
                       schedulerSweeperAllowedOverrun->value())) {
                   QMessageBox::warning(this,tr("Scheduler sweepers"),
                                        tr("The common sweeper folder does not exist."));
                   return;
               }
               planner->setDevicePlay(selectedPlay);
               m_volumeCommitted = true;
               emit planner->configurationChanged();
           }

           this->hide();
       });




}







FrameOptionsPlayer::~FrameOptionsPlayer(){}




void FrameOptionsPlayer::UpdateDevice(){

       comboplay->clear(); // borramos antes para no duplicar
       if (combocue)
           combocue->clear();


    for (const AudioDevice &device : MediaManager::outputDevices()) {
        // Añadimos el dispositivo a ambos combos.
        comboplay->addItem(device.name, device.id);
        if (combocue && !combocue->isHidden())
            combocue->addItem(device.name, device.id);
    }

      // ----------------------------------------
          // Obtener Player propietario del panel
          // ----------------------------------------

          Player *player = qobject_cast<Player*>(this->parent());
          Planner *planner = qobject_cast<Planner*>(this->parent());

          if (!player && !planner)
              return;


          // ----------------------------------------
          // Recuperar configuración actual
          // ----------------------------------------

          int currentPlay = player ? player->devicePlay() : planner->devicePlay();
          int currentCue  = player ? player->deviceCue() : 0;


          // ----------------------------------------
          // Seleccionar dispositivo PLAY
          // ----------------------------------------

          int playComboIndex =
              comboplay->findData(currentPlay);

          if (playComboIndex >= 0)
              comboplay->setCurrentIndex(playComboIndex);


          // ----------------------------------------
          // Seleccionar dispositivo CUE
          // ----------------------------------------

          if (combocue && !combocue->isHidden()) {
              int cueComboIndex=combocue->findData(currentCue);
              if (cueComboIndex>=0)
                  combocue->setCurrentIndex(cueComboIndex);
          }



}





// cada vez que se abre se actualiza
void FrameOptionsPlayer::showEvent(QShowEvent *event) {
    Frame::showEvent(event); // Llama al evento base
    this->UpdateDevice();    // Refresca la lista de dispositivos automáticamente al abrir
    m_volumeCommitted = false;
    if (auto *player = qobject_cast<Player *>(parentWidget())) {
        m_previousVolume = player->volume();
        const QSignalBlocker blocker(volumeSlider);
        const int value = qRound(m_previousVolume * 100);
        volumeSlider->setValue(value);
        volumeValue->setText(QString::number(value) + " %");
    } else if (auto *planner = qobject_cast<Planner *>(parentWidget())) {
        m_previousVolume = planner->volume();
        const QSignalBlocker blocker(volumeSlider);
        const int value = qRound(m_previousVolume * 100);
        volumeSlider->setValue(value);
        volumeValue->setText(QString::number(value) + " %");
        if (schedulerSweeperFolder) {
            schedulerSweeperFolder->setText(planner->schedulerSweeperFolder());
            schedulerSweeperMaximumWindow->setValue(
                planner->schedulerSweeperMaxWindowSeconds());
            schedulerSweeperAllowedOverrun->setValue(
                planner->schedulerSweeperOverrunSeconds());
        }
    }
}

void FrameOptionsPlayer::hideEvent(QHideEvent *event)
{
    if (!m_volumeCommitted) {
        if (auto *player = qobject_cast<Player *>(parentWidget()))
            player->setVolume(m_previousVolume);
        else if (auto *planner = qobject_cast<Planner *>(parentWidget()))
            planner->setVolume(m_previousVolume);
    }
    Frame::hideEvent(event);
}

bool FrameOptionsPlayer::eventFilter(QObject *watched, QEvent *event)
{
    if (m_isPlannerOptions && isVisible() && !m_folderDialogOpen) {
        if (event->type()==QEvent::MouseButtonPress) {
            auto *clicked=qobject_cast<QWidget *>(watched);
            if (clicked && clicked!=this && !isAncestorOf(clicked))
                hide();
        } else if (event->type()==QEvent::KeyPress) {
            auto *keyEvent=static_cast<QKeyEvent *>(event);
            if (keyEvent->key()==Qt::Key_Escape)
                hide();
        }
    }
    return Frame::eventFilter(watched,event);
}

float FrameOptionsPlayer::committedVolume() const
{
    if (isVisible() && !m_volumeCommitted)
        return m_previousVolume;
    const auto *player = qobject_cast<Player *>(parentWidget());
    if (player)
        return player->volume();
    const auto *planner = qobject_cast<Planner *>(parentWidget());
    return planner ? planner->volume() : 1.0f;
}
