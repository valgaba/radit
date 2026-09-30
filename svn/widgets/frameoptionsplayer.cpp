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

#include "widgets/frameoptionsplayer.h"
#include "widgets/label.h"
#include "widgets/Player.h"
#include "bass.h"




FrameOptionsPlayer::FrameOptionsPlayer(QWidget *parent):Frame(parent){

    this->setObjectName("FrameOptionsPlayer"); //para qss
    this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    this->setFixedSize(350, 250);

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

    //parte alta

    frametop->setStyleSheet("background-color: #4e4d7a; border: none;");


     Label * labeltext = new Label(this);
     labeltext->setMaximumHeight(QWIDGETSIZE_MAX);
     labeltext->setWordWrap(false);
     QFont font;
          font.setPointSize(10);
          font.setBold(true);
          labeltext->setFont(font);

     labeltext->setText("Player options");
     layouttop->addWidget(labeltext);


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

    // this->UpdateDevice();

  /*  BASS_DEVICEINFO info;
      for (int i = 0; BASS_GetDeviceInfo(i, &info); i++) {
          if (info.flags & BASS_DEVICE_ENABLED) {
              QString deviceName = QString::fromUtf8(info.name);

              // Añadimos el dispositivo a ambos combos
              comboplay->addItem(deviceName, i);
              combocue->addItem(deviceName, i);
          }
      }*/

    layoutcenter->addRow(labelplay, comboplay);
    layoutcenter->addRow(labelcue, combocue);









    //parte baja

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

    layoutdown->addWidget(btncancel);
    layoutdown->addWidget(btnacept);


    // Cancelar -> ocultar el panel
    connect(btncancel, &QPushButton::clicked,
            this, &FrameOptionsPlayer::hide);



    // aceptar ->
    connect(btnacept, &QPushButton::clicked, this, [this]() {

           int selectedPlay = comboplay->currentData().toInt();
           int selectedCue = combocue->currentData().toInt();

           // 2. Intentar hacer un cast seguro al tipo de tu clase padre
           // Reemplaza "TuClasePadre" por el nombre real de la clase que tiene los métodos set
          Player *player = qobject_cast<Player*>(this->parent());

           if (player) {
               this->ApplySelectedDevices();
               player->setDevicePlay(selectedPlay);
               player->setDeviceCue(selectedCue);

           }


           this->hide();
       });




}







FrameOptionsPlayer::~FrameOptionsPlayer(){}




void FrameOptionsPlayer::UpdateDevice(){

       comboplay->clear(); // borramos antes para no duplicar
       combocue->clear();


    BASS_DEVICEINFO info;
      for (int i = 0; BASS_GetDeviceInfo(i, &info); i++) {
          if (info.flags & BASS_DEVICE_ENABLED) {
              QString deviceName = QString::fromUtf8(info.name);

              // Añadimos el dispositivo a ambos combos
              comboplay->addItem(deviceName, i);
              combocue->addItem(deviceName, i);

            /*  BASS_SetDevice(i);
              BASS_Free();

              if (!BASS_Init(i, 44100, 0, nullptr, nullptr))
              {
                  qDebug() << "BASS_Init error:" << BASS_ErrorGetCode();

              }*/


          }
      }

}


// Para poner y quitar USB en caliente
void FrameOptionsPlayer::ApplySelectedDevices() {
    // Obtenemos los índices que el usuario tiene seleccionados en la interfaz gráfica
    int playDevice = comboplay->currentData().toInt();
    int cueDevice = combocue->currentData().toInt();

    // Inicializamos el dispositivo de REPRODUCCIÓN (Play)
    if (playDevice >= 0) {
        BASS_SetDevice(playDevice);
        BASS_Free(); //  libera el estado "fantasma" del USB anterior
        if (!BASS_Init(playDevice, 44100, 0, nullptr, nullptr)) {
            qDebug() << "BASS_Init Play error:" << BASS_ErrorGetCode();
        }
    }

    // Inicializamos el dispositivo de PREESCUCHA (Cue) si es diferente al de Play
    if (cueDevice >= 0 && cueDevice != playDevice) {
        BASS_SetDevice(cueDevice);
        BASS_Free();
        if (!BASS_Init(cueDevice, 44100, 0, nullptr, nullptr)) {
            qDebug() << "BASS_Init Cue error:" << BASS_ErrorGetCode();
        }
    }
}


// cada vez que se abre se actualiza
void FrameOptionsPlayer::showEvent(QShowEvent *event) {
    Frame::showEvent(event); // Llama al evento base
    this->UpdateDevice();    // Refresca la lista de dispositivos automáticamente al abrir
}
