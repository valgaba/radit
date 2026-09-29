#ifndef FRAMEOPTIONSPLAYER_H
#define FRAMEOPTIONSPLAYER_H


#include "widgets/frame.h"

#include <QFormLayout>
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


    public:



      explicit FrameOptionsPlayer( QWidget *parent = 0);
      ~FrameOptionsPlayer();





    protected:




    private slots:


};





#endif // FRAMEOPTIONSPLAYER_H
