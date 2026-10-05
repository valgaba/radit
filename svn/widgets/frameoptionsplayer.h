#ifndef FRAMEOPTIONSPLAYER_H
#define FRAMEOPTIONSPLAYER_H


#include "widgets/frame.h"

#include <QFormLayout>
#include <QComboBox>
#include "widgets/button.h"

class QSlider;
class QLabel;




class FrameOptionsPlayer: public Frame

{
    Q_OBJECT

    private:

    Frame *frametop;
    Frame *framecenter;
    Frame *framedown;

     QVBoxLayout *layout; //general
     QHBoxLayout *layouttop;
     QFormLayout *layoutcenter; //general
     QHBoxLayout *layoutdown;


    Button * btncancel;
    Button * btnacept;
    QComboBox *comboplay;
    QComboBox *combocue;
    QSlider *volumeSlider;
    QLabel *volumeValue;
    float m_previousVolume = 1.0f;
    bool m_volumeCommitted = false;




    public:



      explicit FrameOptionsPlayer( QWidget *parent = 0);
      ~FrameOptionsPlayer();


       void UpdateDevice();
       float committedVolume() const;


    protected:

   void showEvent(QShowEvent *event) override;
   void hideEvent(QHideEvent *event) override;


    private slots:


};





#endif // FRAMEOPTIONSPLAYER_H
