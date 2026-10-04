/* This file is part of Radit.
  Copyright 2022, Victor Algaba <victorengine@gmail.com> www.radit.org

  Radit is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  radit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with radit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QDateTime>
#include <QSpacerItem>



#include "widgets/AudioItemMaxi.h"
#include "widgets/Player.h"
#include "widgets/CueWaveformFrame.h"


AudioItemMaxi::AudioItemMaxi(QWidget *parent):AudioItem(parent){


   this->setObjectName("AudioItemMaxi"); //para qss
   this->setAttribute(Qt::WA_StyledBackground, true);
   this->setFixedHeight(75); //alto del item sin botonera inferior
   //this->setStyleSheet("background-color: #262c3b;");


     mediamanager = new MediaManager(this);

     connect(mediamanager, &MediaManager::audioFrameUpdated,
             this, [this](const AudioFrame &frame) {
         setSecondStart(frame.position);
     });

     connect(mediamanager, &MediaManager::playbackFinished, this, [this]() {
         mediamanager->seek(0.0);
         mediamanager->stop();
     });
//***************************************************


   layout = new QVBoxLayout; //layout general
   layout->setContentsMargins(2, 2, 2, 2);
   layout->setSpacing(0); // espacios entre  item dentro del contenedor
   this->setLayout(layout);

   // dividimos en dos partes

       frametop = new Frame;
       framecenter = new Frame;



       //layou de las zonas
       layouttop = new QHBoxLayout;
       layoutcenter = new QHBoxLayout;


       layoutcenterleft = new QHBoxLayout;
       layoutcenterright = new QHBoxLayout ;



       layouttop->setContentsMargins(0, 0, 0, 0);
       layoutcenter->setContentsMargins(0, 0, 0, 0);
       layoutcenter->setSpacing(0); // espacios entre  item dentro del contenedor

       layoutcenterleft->setContentsMargins(0, 0, 0, 0);
       layoutcenterright->setContentsMargins(0, 0, 0, 0);
       layoutcenterleft->setSpacing(0); // espacios entre  item dentro del contenedor
       layoutcenterright->setSpacing(0); // espacios entre  item dentro del contenedor



       frametop->setLayout(layouttop);
       framecenter->setLayout(layoutcenter);



       // la zona centro se divide en dos partes con sus correspondientes layou
       framecenterleft = new Frame;
       framecenterright = new Frame;

       framecenterleft->setLayout(layoutcenterleft);
       framecenterright->setLayout(layoutcenterright);

       framecenterleft->setStyleSheet("Frame { border: none;}"); //quitamos bordes de los layou
       framecenterright->setStyleSheet("Frame { border: none;}");


       layoutcenter->addWidget(framecenterleft);
       layoutcenter->addWidget(framecenterright,1); // solo se redimensiona este


//****************Botonera parte alta*************

       framecolor = new FrameColorItemMax;

       btnselect = new Button;
      // btnproperties->SetIcon("GuiTabMenu.svg");
       btnselect->setFixedSize(21, 21);  //Tamaño fijo
       btnselect->setToolTip("Select item");


        btnproperties = new Button;
        btnproperties->SetIcon("more.svg");
        btnproperties->setIconSize(QSize(18, 18));
        btnproperties->setFixedSize(21, 21);  //Tamaño fijo
        btnproperties->setToolTip("Properties");

        btndelete = new Button;
        btndelete->SetIcon("Remove.svg");
        btndelete->setIconSize(QSize(18, 18));
        btndelete->setFixedSize(21, 21);  //Tamaño fijo
        btndelete->setToolTip("Delete item");


        btnloop = new Button;
        btnloop->SetIcon("Loop.svg");
        btnloop->setIconSize(QSize(18, 18));   // no termina de gustarme el tamaño del icono por defecto
        btnloop->setFixedSize(21, 21);  //Tamaño fijo
        btnloop->setToolTip("Repeat the item indefinitely.");

        btnpurge = new Button;
        btnpurge->SetIcon("Purge.svg");
        btnpurge->setIconSize(QSize(18, 18));   // no termina de gustarme el tamaño del icono por defecto
        btnpurge->setFixedSize(21, 21);  //Tamaño fijo 30,29
        btnpurge->setToolTip("Delete the item once it has been played.");

        btnnext = new Button;
        btnnext->SetIcon("Next.svg");
        btnnext->setIconSize(QSize(18, 18));   // no termina de gustarme el tamaño del icono por defecto
        btnnext->setFixedSize(21, 21);  //Tamaño fijo
        btnnext->setToolTip("Play the item once the previous one has finished");


        connect(btnnext, &QPushButton::clicked, this, [=](){
             this->setIsPlayNext(!this->isPlayNext());
             //this->mediamanager->fadeOut(2000);
        });

        connect(btnpurge, &QPushButton::clicked, this, [=](){
             this->setIsPurge(!this->isPurge());

        });

        connect(btnloop, &QPushButton::clicked, this, [=](){
             this->setIsLoop(!this->isLoop());

        });

        connect(btnselect, &QPushButton::clicked, this, [=](){
             this->setIsSelect(!this->isSelect());
        });




        connect(btndelete, &QPushButton::clicked,
                this, &AudioItemMaxi::onDeleteClicked);


        layouttop->addWidget(framecolor, 0, Qt::AlignTop); //añadimos un color al item
        layouttop->addItem(new QSpacerItem(363, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum)); //espaciador

        // añadimos botones a la parte alta
        layouttop->addWidget(btnnext);
        layouttop->addWidget(btnpurge);
        layouttop->addWidget(btnloop);
        layouttop->addWidget(btndelete);
        layouttop->addWidget(btnproperties);
        layouttop->addWidget(btnselect);



        // botonera parte centro**********************************

        framecenter->setFixedHeight(43);  // prueba 35–50
        btnplay = new Button;
        btnplay->SetIcon("playpause2.svg");
        btnplay->setIconSize(QSize(40, 50));  // ajusta al tamaño que quieras
        btnplay->setFixedSize(50, 30);  //Tamaño fijo 35
        btnplay->setToolTip("Play");


        connect(btnplay, &QPushButton::clicked, this, [this]() {
            emit requestPlay(this);
        });



        labelnombre = new Label;
        labelnombre->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        labelnombre->setText("fichero.....dfdfdfdfdfdfdfdfdfdfdf.....");

        labeltiempo = new Label;
        labeltiempo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        labeltiempo->setFixedWidth(125);   // Fija solo el ancho
        // Cambiar tamaño de fuente
        QFont font = labeltiempo->font();
        font.setPointSize(16); //16
        font.setBold(true);
        labeltiempo->setFont(font);

        QFont fontnombre = labelnombre->font();
        fontnombre.setPointSize(13); //16
        fontnombre.setBold(true);
        labelnombre->setFont(fontnombre);




        labeltiempo->setText("00:03:12.00");

        labeltiempo->setFixedHeight(35); //45
        labelnombre->setFixedHeight(45);

        layoutcenterleft->addWidget(btnplay);
        layoutcenterright->addWidget(labelnombre);
        layoutcenterright->addWidget(labeltiempo);







// Botón para abrir los controles del cue en el waveform.
        auto *btnwaveform = new Button;
        btnwaveform->SetIcon("waveform.svg");
        btnwaveform->setIconSize(QSize(20, 20));
        btnwaveform->setFixedSize(23, 23);
        btnwaveform->setToolTip(tr("Show cue waveform"));
        btnwaveform->setAccessibleName(btnwaveform->toolTip());
        layouttop->insertWidget(layouttop->indexOf(btnproperties) + 1, btnwaveform);
        connect(btnwaveform, &QPushButton::clicked, this, [this]() {
            QWidget *player = parentWidget();
            while (player && !qobject_cast<Player *>(player))
                player = player->parentWidget();
            if (!m_cueWaveform) {
                m_cueWaveform = new CueWaveformFrame(mediamanager, this);
                connect(m_cueWaveform, &CueWaveformFrame::playPauseRequested, this, &AudioItemMaxi::toggleCuePlayback);
                connect(m_cueWaveform, &CueWaveformFrame::stopRequested, this, [this]() {
                    mediamanager->stop();
                    mediamanager->seek(m_cueStartPosition);
                });
                connect(m_cueWaveform, &CueWaveformFrame::seekRequested, this, [this](double seconds) {
                    if (!prepareCue()) return;
                    if (mediamanager->isPlaying()) mediamanager->pause();
                    mediamanager->seek(seconds);
                    m_cueStartPosition = mediamanager->getPosition();
                });
            }
            m_cueWaveform->setWindowTitle(labelnombre->text());
            m_cueWaveform->showWaveform(filePath(), player ? player : this);
        });
        //añadimos al principal

        layout->addWidget(frametop,1);
        layout->addWidget(framecenter,1);


}



void AudioItemMaxi::toggleCuePlayback()
{
    if (mediamanager->isPlaying()) {
        mediamanager->pause();
        return;
    }
    if (!prepareCue()) return;
    if (!mediamanager->isPaused())
        m_cueStartPosition = mediamanager->getPosition();
    mediamanager->play();
}

bool AudioItemMaxi::prepareCue()
{
    const int device = devicePlay();
    const bool sameFile = m_loadedCuePath == filePath() && !m_loadedCuePath.isEmpty();
    if (sameFile && m_loadedCueDevice == device)
        return true;
    const double position = sameFile ? mediamanager->getPosition() : 0;
    const bool playing = sameFile && mediamanager->isPlaying();
    const bool paused = sameFile && mediamanager->isPaused();
    if (!mediamanager->setDevice(device) || !mediamanager->loadFile(filePath()))
        return false;
    m_loadedCuePath = filePath();
    m_loadedCueDevice = device;
    if (!sameFile) m_cueStartPosition = 0;
    if (sameFile) {
        mediamanager->seek(position);
        if (playing || paused) mediamanager->play();
        if (paused) mediamanager->pause();
    }
    return true;
}

AudioItemMaxi::~AudioItemMaxi(){}



//****************************************************
void AudioItemMaxi::setNameFile(const QString &nombre)
{
    m_NameFile=nombre;

    this->labelnombre->setText(nombre);
    if (m_cueWaveform) m_cueWaveform->setWindowTitle(nombre);
}


void AudioItemMaxi::setTiempoFile(double segundos){
        this->labeltiempo->setText(SecondToTime(segundos));
}



const QString& AudioItemMaxi::nameFile() const
{
    return m_NameFile;
}


QString AudioItemMaxi::SecondToTime(double segundos){

    if (segundos < 0) {
          return "00:00:00.00";
        }

        // Convertimos a milisegundos para mayor precisión
      //  qint64 totalMilliseconds = static_cast<qint64>(segundos * 1000.0);
        qint64 totalMilliseconds = qRound64(segundos * 1000.0);

        int horas = totalMilliseconds / 3600000;
        int minutos = (totalMilliseconds % 3600000) / 60000;
        int seg = (totalMilliseconds % 60000) / 1000;
        int centesimas = (totalMilliseconds % 1000) / 10;

        QString tiempo = QString("%1:%2:%3.%4")
                .arg(horas, 2, 10, QChar('0'))
                .arg(minutos, 2, 10, QChar('0'))
                .arg(seg, 2, 10, QChar('0'))
                .arg(centesimas, 2, 10, QChar('0'));

        return tiempo;



}

void AudioItemMaxi::setIsSelect(bool value)
{
    AudioItem::setIsSelect(value); // guardar estado en la base

    btnselect->setProperty("selectcolor", isSelect());
    btnselect->style()->polish(btnselect);
    btnselect->update();
}




//***************************************
void AudioItemMaxi::onDeleteClicked(){  // pulsamos boton borrar

    emit requestDelete(this);  //mandamos señal borrar
}




bool AudioItemMaxi::isPlaying() const {
    return m_isPlaying;
}

void AudioItemMaxi::setPlaying(bool playing) {

    m_isPlaying = playing;

}


void AudioItemMaxi::playColor(bool playing){

    btnplay->setProperty("playing", playing);
    btnplay->style()->unpolish(btnplay);
    btnplay->style()->polish(btnplay);
    btnplay->update();


    // ITEM COMPLETO
      this->setProperty("playing", playing);
      this->style()->unpolish(this);
      this->style()->polish(this);
      this->update();


    if (playing) {
           btnplay->SetIcon("playpause2on.svg");
           btnplay->setIconSize(QSize(40, 50));

       } else {
           btnplay->SetIcon("playpause2.svg");
           btnplay->setIconSize(QSize(40, 50));


       }


}



int AudioItemMaxi::devicePlay() const {

    QWidget *widget = this->parentWidget();
    Player *player = nullptr;

    while (widget) {
        player = qobject_cast<Player*>(widget);
        if (player) {
            break; // ¡Lo encontramos!
        }
        widget = widget->parentWidget(); // Seguimos subiendo
    }

    // 2. Si lo encontramos, le pedimos el dispositivo asignado
    if (player){
           return player->deviceCue();
    }

    return 0;

}


void AudioItemMaxi::setColor(const QColor& color)
{
    framecolor->setColor(color);
}

QColor AudioItemMaxi::color() const
{
    return framecolor->color();
}
