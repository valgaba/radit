#ifndef AUDIOITEMMAXI_H
#define AUDIOITEMMAXI_H


#include <QVBoxLayout>
#include <QHBoxLayout>


#include "widgets/frame.h"
#include "widgets/AudioItem.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/FrameColorItemMax.h"
#include "widgets/slider.h"



class AudioItemMaxi: public AudioItem{


    Q_OBJECT


private:


    QVBoxLayout *layout; //general

    FrameColorItemMax *framecolor;
    Frame *frametop;
    Frame *framecenter;
    Frame *framedown;

    Frame *framecenterleft;
    Frame *framecenterright;

    QHBoxLayout *layouttop;
    QHBoxLayout *layoutdown;
    QHBoxLayout *layoutcenter;


    QHBoxLayout *layoutcenterleft;
    QHBoxLayout *layoutcenterright;



    Button * btnproperties;
    Button * btndelete;
    Button * btnselect;
    Button * btnloop;
    Button * btnpurge;
    Button * btnnext;



    Button * btnplay;
    Label *labelnombre;
    Label *labeltiempo;



    Button * btnplaycue;
    Button * btnrewind;
    Button * btnforward;

    Slider * slider;


     bool m_isPlayNext=false;
     bool m_isPurge=false;
     bool m_isLoop=false;
     bool m_isSelect=false;


public:

    void setNameFile(const QString &NameFile) const;


    explicit AudioItemMaxi(QWidget *parent = 0);
    ~AudioItemMaxi();


     virtual AudioItemMaxi* copy(QWidget* newParent) const = 0;

    void setIsPlayNext(bool isPlayNext) {
           m_isPlayNext = isPlayNext;

           if (btnnext) {
                   if (m_isPlayNext) {
                       btnnext->setStyleSheet("background-color: #39385b;");
                   } else {
                       btnnext->setStyleSheet("background-color: none; color: black;");
                   }
               }
       }

    bool isPlayNext() const {
          return m_isPlayNext;
       }


    void setIsPurge(bool isPurge) {
           m_isPurge = isPurge;

           if (btnpurge) {
                   if (m_isPurge) {
                       btnpurge->setStyleSheet("background-color: #39385b;");
                   } else {
                       btnpurge->setStyleSheet("background-color: none; color: black;");
                   }
               }
       }

    bool isPurge() const {
          return m_isPurge;
       }



    void setIsLoop(bool isLoop) {
           m_isLoop = isLoop;

           if (btnloop) {
                   if (m_isLoop) {
                       btnloop->setStyleSheet("background-color: #39385b;");
                   } else {
                       btnloop->setStyleSheet("background-color: none; color: black;");
                   }
               }


           // Control de visibilidad
              if (btnpurge) {
                  btnpurge->setVisible(!m_isLoop);
              }

              if (btnnext) {
                  btnnext->setVisible(!m_isLoop);
              }




       }

    bool isLoop() const {
          return m_isLoop;
       }



    protected:


    private slots:
      void onDeleteClicked();   // ← SLOT aquí


    public slots:


    signals:
      void requestDelete(AudioItemMaxi* item);




};



#endif // AUDIOITEMMAXI_H
