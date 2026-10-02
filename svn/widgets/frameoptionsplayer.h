#ifndef FRAMEOPTIONSPLAYER_H
#define FRAMEOPTIONSPLAYER_H


#include "widgets/frame.h"

#include <QFormLayout>
#include <QComboBox>
#include "widgets/button.h"




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




    public:



      explicit FrameOptionsPlayer( QWidget *parent = 0);
      ~FrameOptionsPlayer();


       void UpdateDevice();


    protected:

   void showEvent(QShowEvent *event) override;


    private slots:


};





#endif // FRAMEOPTIONSPLAYER_H
