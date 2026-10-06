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
#include "core/MediaManager.h"

class CueWaveformFrame;
class QTimer;
class QGraphicsOpacityEffect;



class AudioItemMaxi: public AudioItem{


    Q_OBJECT


private:


    QVBoxLayout *layout; //general


    Frame *frametop;
    Frame *framecenter;

    Frame *framecenterleft;
    Frame *framecenterright;

    QHBoxLayout *layouttop;
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



    CueWaveformFrame *m_cueWaveform = nullptr;
    Button *m_cueButton = nullptr;
    QTimer *m_cueBlinkTimer = nullptr;
    QGraphicsOpacityEffect *m_cueOpacity = nullptr;
    void updateCueIndicator();


     bool m_isPlayNext=false;
     bool m_isPurge=false;
     bool m_isLoop=false;
    // bool m_isSelect=false;
     QString m_NameFile;

     MediaManager * mediamanager;

     double m_cueStartPosition = 0.0;
     QString m_loadedCuePath;
     int m_loadedCueDevice = -2;
     bool prepareCue();
     void toggleCuePlayback();

     QString SecondToTime(double segundos);

     double m_secondstart = 0;

     bool m_isPlaying = false;

     int devicePlay() const;

public:

      FrameColorItemMax *framecolor; // esto es para cambiar

    void playColor(bool playing);
    void setNameFile(const QString &NameFile);
    void setTiempoFile(double segundos);

    const QString& nameFile() const;


    explicit AudioItemMaxi(QWidget *parent = 0);
    ~AudioItemMaxi();


     virtual AudioItemMaxi* copy(QWidget* newParent) const = 0;
     virtual bool isLiveStream() const { return false; }
     virtual bool advancesOnLoop() const { return false; }
     virtual bool preparePlayback() { return true; }
     virtual QString playbackPath() const { return filePath(); }
     virtual QString playbackName() const { return nameFile(); }
     void setIsSelect(bool value) override;


    void setIsPlayNext(bool isPlayNext) {
        m_isPlayNext = isPlayNext;

            btnnext->setProperty("nextcolor", m_isPlayNext);
            btnnext->style()->polish(btnnext);
            btnnext->update();

            if (m_isPlayNext) {
                   btnnext->SetIcon("Nexton.svg");
                   btnnext->setIconSize(QSize(18, 18));
               } else {
                   btnnext->SetIcon("Next.svg");
                   btnnext->setIconSize(QSize(18, 18));

               }

       }

    bool isPlayNext() const {
          return m_isPlayNext;
       }


    void setIsPurge(bool isPurge) {
           if (isLiveStream()) isPurge = false;
           m_isPurge = isPurge;
           btnpurge->setProperty("purgecolor", m_isPurge); //active viene del css
           btnpurge->style()->polish(btnpurge);
           btnpurge->update();

           if (m_isPurge) {
                  btnpurge->SetIcon("Purgeon.svg");
                  btnpurge->setIconSize(QSize(18, 18));
              } else {
                  btnpurge->SetIcon("Purge.svg");
                  btnpurge->setIconSize(QSize(18, 18));

              }


       }

    bool isPurge() const {
          return m_isPurge;
       }



    void setIsLoop(bool isLoop) {
           if (isLiveStream()) isLoop = false;
           m_isLoop = isLoop;

           btnloop->setProperty("loopcolor", m_isLoop); //active viene del css
           btnloop->style()->polish(btnloop);
           btnloop->update();


           if (m_isLoop) {
                  btnloop->SetIcon("Loopon.svg");
                  btnloop->setIconSize(QSize(18, 18));
              } else {
                  btnloop->SetIcon("Loop.svg");
                  btnloop->setIconSize(QSize(18, 18));

              }


           // Control de visibilidad
              if (btnpurge) {
                  btnpurge->setVisible(!m_isLoop && !isLiveStream());
              }

              if (btnnext) {
                  btnnext->setVisible(!m_isLoop);
              }




       }

    bool isLoop() const {
          return m_isLoop;
       }



    void setSecondStart(double secondStart);

    double secondStart() const {
        return m_secondstart;
    }


    void setPlaying(bool playing);
    bool isPlaying() const;

    void setColor(const QColor& color);
    QColor color() const;


    protected:
    void setLiveStreamPresentation();
    void setFolderPresentation(const QString &status, bool ready);
    Button *propertiesButton() const { return btnproperties; }
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;


    private slots:
      void onDeleteClicked();   // ← SLOT aquí


    public slots:


    signals:
      void requestDelete(AudioItemMaxi* item); //pregunta antes
      void requestAutoDelete(AudioItemMaxi* item); //no pregunta
      void requestPlay(AudioItemMaxi *item);




};



#endif // AUDIOITEMMAXI_H
