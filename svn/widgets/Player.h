#ifndef PLAYER_H
#define PLAYER_H



#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>

#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/frame.h"
#include "widgets/TabPlayer.h"
#include "widgets/AudioItemMaxi.h"
#include "core/MediaManager.h"



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



    Button * btnclose;

    Label *labeltitle;

    TabPlayer *tabplayer;

    MediaManager *mediamanager;
    AudioItemMaxi *currentItem;


public:

    explicit Player(QWidget *parent = nullptr);
    ~Player();

    void setTitle(QString title);
    QString title() const;

    void playItem(AudioItemMaxi *item);
    void pauseMain();
    void stopMain();

    protected:


    private slots:


    public slots:


    signals:




};





#endif // PLAYER_H
