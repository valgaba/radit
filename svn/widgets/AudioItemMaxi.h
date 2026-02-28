#ifndef AUDIOITEMMAXI_H
#define AUDIOITEMMAXI_H


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>

#include "widgets/frame.h"
#include "widgets/AudioItem.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/FrameColorItemMax.h"
#include "widgets/slider.h"
#include "core/MediaManager.h"



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
    Button * btnstopcue;
    Button * btnrewind;
    Button * btnforward;
    Label *labeltiempocue;

    Slider * slider;


     bool m_isPlayNext=false;
     bool m_isPurge=false;
     bool m_isLoop=false;
     bool m_isSelect=false;
     QString m_NameFile;

     MediaManager * mediamanager;

     bool m_userIsSeeking = false;
     double m_duration = 0.0;

     QString SecondToTime(double segundos);


public:

    void setNameFile(const QString &NameFile);
    void setTiempoFile(double segundos);

    const QString& nameFile() const;


    explicit AudioItemMaxi(QWidget *parent = 0);
    ~AudioItemMaxi();


     virtual AudioItemMaxi* copy(QWidget* newParent) const = 0;

    void setIsPlayNext(bool isPlayNext) {
        m_isPlayNext = isPlayNext;

            btnnext->setProperty("active", m_isPlayNext);
            btnnext->style()->polish(btnnext);
            btnnext->update();
       }

    bool isPlayNext() const {
          return m_isPlayNext;
       }


    void setIsPurge(bool isPurge) {
           m_isPurge = isPurge;
           btnpurge->setProperty("active", m_isPurge); //active viene del css
           btnpurge->style()->polish(btnpurge);
           btnpurge->update();
       }

    bool isPurge() const {
          return m_isPurge;
       }



    void setIsLoop(bool isLoop) {
           m_isLoop = isLoop;

           btnloop->setProperty("active", m_isLoop); //active viene del css
           btnloop->style()->polish(btnloop);
           btnloop->update();

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




    void setIsSelect(bool isSelect) {
           m_isSelect = isSelect;
           btnselect->setProperty("active", m_isSelect); //active viene del css
           btnselect->style()->polish(btnselect);
           btnselect->update();

       }

    bool isSelect() const {
          return m_isSelect;
       }






    protected:


    private slots:
      void onDeleteClicked();   // ← SLOT aquí


    public slots:


    signals:
      void requestDelete(AudioItemMaxi* item);




};



#endif // AUDIOITEMMAXI_H
