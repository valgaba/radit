#ifndef PLAYER_H
#define PLAYER_H



#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>

#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/frame.h"
#include "widgets/TabPlayer.h"
#include "widgets/container.h"



class Player: public Frame{


    Q_OBJECT


private:


    QVBoxLayout *layout; //general

   // dividimos en 4 partes
    Frame *framebarra;
    Frame *frametop;
    Frame *framecenter;
    Frame *framedown;
    Frame *frametab;

    QHBoxLayout *layoutbarra;
    QHBoxLayout *layouttop;
    QHBoxLayout *layoutcenter;
    QHBoxLayout *layoutdown;
    QVBoxLayout *layouttab;



    Button * btnproperties;

    Label *labelnombre;

    TabPlayer *tabplayer;


public:

    explicit Player(QWidget *parent = nullptr);
    ~Player();



    protected:


    private slots:


    public slots:


    signals:




};





#endif // PLAYER_H
