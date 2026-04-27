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


AudioItemMaxi::AudioItemMaxi(QWidget *parent):AudioItem(parent){


   this->setObjectName("AudioItemMaxi"); //para qss
   this->setFixedHeight(100); //alto del item fijo
   //this->setStyleSheet("background-color: #262c3b;");
     mediamanager = new MediaManager(this);

     connect(mediamanager, &MediaManager::audioFrameUpdated,
             this, [this](const AudioFrame &frame) {


         this->labeltiempocue->setText(SecondToTime(m_duration-frame.position)); //cuenta atras del tiempo
         this->setSecondStart(frame.position); //guardamos los segundos de pause para luego el play

         if (!m_userIsSeeking && m_duration > 0.0)
         {
             int value = static_cast<int>((frame.position / m_duration) * 1000.0);
             slider->setValue(value);
         }

     });



     connect(mediamanager, &MediaManager::playbackFinished,
             this, [this]()
     {
         mediamanager->seek(0.0);   // volver al inicio
         mediamanager->stop();
         m_pauseBlinkTimer->stop();
         labeltiempocue->setStyleSheet("color: #84868f;");

     });




     m_pauseBlinkTimer = new QTimer(this);
     m_pauseBlinkTimer->setInterval(500); // parpadeo cada 500ms

     connect(m_pauseBlinkTimer, &QTimer::timeout, this, [this]() {
         m_labelVisible = !m_labelVisible;
            if (m_labelVisible) {
                labeltiempocue->setStyleSheet("color: #84868f;");
            } else {
                labeltiempocue->setStyleSheet("color: transparent;"); // texto invisible pero el espacio se mantiene
            }
     });






   layout = new QVBoxLayout; //layout general
   layout->setContentsMargins(0, 0, 0, 0);
   layout->setSpacing(0); // espacios entre  item dentro del contenedor
   this->setLayout(layout);

   // diviedimos en tres partes

       frametop = new Frame;
       framecenter = new Frame;
       framedown = new Frame;



       //layou de las zonas
       layouttop = new QHBoxLayout;
       layoutdown =new QHBoxLayout;
       layoutcenter = new QHBoxLayout;


       layoutcenterleft = new QHBoxLayout;
       layoutcenterright = new QHBoxLayout ;



       layouttop->setContentsMargins(0, 0, 0, 0);
       layoutdown->setContentsMargins(0, 0, 0, 0);
       layoutcenter->setContentsMargins(0, 0, 0, 0);
       layoutcenter->setSpacing(0); // espacios entre  item dentro del contenedor

       layoutcenterleft->setContentsMargins(0, 0, 0, 0);
       layoutcenterright->setContentsMargins(0, 0, 0, 0);
       layoutcenterleft->setSpacing(0); // espacios entre  item dentro del contenedor
       layoutcenterright->setSpacing(0); // espacios entre  item dentro del contenedor



       frametop->setLayout(layouttop);
       framecenter->setLayout(layoutcenter);
       framedown->setLayout(layoutdown);



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







///***************** botonera parte baja ****************
        btnplaycue = new Button;
        btnplaycue->SetIcon("playpause.svg");
        btnplaycue->setIconSize(QSize(25, 25));   // no termina de gustarme el tamaño del icono por defecto
        btnplaycue->setFixedSize(23, 23);  //Tamaño fijo
        btnplaycue->setToolTip("Properties");

        btnstopcue = new Button;
        btnstopcue->SetIcon("Stop.svg");
        btnstopcue->setIconSize(QSize(25, 25));   // no termina de gustarme el tamaño del icono por defecto
        btnstopcue->setFixedSize(23, 23);  //Tamaño fijo
        btnstopcue->setToolTip("Stop cue");


        btnrewind = new Button;
        btnrewind->SetIcon("rewind.svg");
        btnrewind->setIconSize(QSize(20, 20));
        btnrewind->setFixedSize(23, 23);  //Tamaño fijo
        btnrewind->setToolTip("Properties");

        btnforward = new Button;
        btnforward->SetIcon("forward.svg");
        btnforward->setIconSize(QSize(20, 20));
        btnforward->setFixedSize(23, 23);  //Tamaño fijo
        btnforward->setToolTip("Properties");


         slider = new Slider;


         labeltiempocue = new Label;
         labeltiempocue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
         labeltiempocue->setFixedWidth(125);   // Fija solo el ancho
         font.setPointSize(14);
         labeltiempocue->setFont(font);
         labeltiempocue->setText("00:00:00.00");

        layoutdown->addWidget(btnplaycue);
        layoutdown->addWidget(btnstopcue);

        layoutdown->addWidget(btnrewind);
        layoutdown->addWidget(btnforward);
        layoutdown->addWidget(slider,1);
        layoutdown->addWidget(labeltiempocue);

        connect(btnplaycue, &QPushButton::clicked, this, [=](){

            if (!mediamanager->isPlaying() && !mediamanager->isPaused()){

                   mediamanager->setDevice(0);
                   mediamanager->loadFile(this->filePath());
                   m_duration = this->second();
                   mediamanager->play();

                   // Detener parpadeo por si estaba activo
                   m_pauseBlinkTimer->stop();
                   labeltiempocue->setStyleSheet("color: #84868f;");
                   return;
               }

               if (mediamanager->isPlaying()){
                   mediamanager->pause();
                   // Iniciar parpadeo
                   m_pauseBlinkTimer->start();
               } else {
                   mediamanager->play();

                   // Detener parpadeo
                   m_pauseBlinkTimer->stop();
                   labeltiempocue->setStyleSheet("color: #84868f;");
               }

        });


        connect(btnstopcue, &QPushButton::clicked,
                this, [=]() {
                mediamanager->stop();
                mediamanager->seek(0.0);

              m_pauseBlinkTimer->stop();
              labeltiempocue->setStyleSheet("color: #84868f;");
                });



        connect(btnrewind, &QPushButton::clicked,
                mediamanager, &MediaManager::rewind);

        connect(btnforward, &QPushButton::clicked,
                mediamanager, &MediaManager::forward);




        connect(slider, &QSlider::sliderPressed, this, [this]() {
            m_userIsSeeking = true;

        });

        connect(slider, &QSlider::sliderReleased, this, [this]() {
            m_userIsSeeking = false;

            double percent = slider->value() / 1000.0;
            mediamanager->seek(percent * m_duration);
        });



        //añadimos al principal

        layout->addWidget(frametop,1);
        layout->addWidget(framecenter,1);
        layout->addWidget(framedown,1);


}



AudioItemMaxi::~AudioItemMaxi(){}



//****************************************************
void AudioItemMaxi::setNameFile(const QString &nombre)
{
    m_NameFile=nombre;

    this->labelnombre->setText(nombre);
}


void AudioItemMaxi::setTiempoFile(double segundos){
        this->labeltiempo->setText(SecondToTime(segundos));
        this->labeltiempocue->setText(SecondToTime(segundos));
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

    btnselect->setProperty("active", isSelect());
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

