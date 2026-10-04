#ifndef PLAYER_H
#define PLAYER_H



#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyle>

#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/slider.h"
#include "widgets/frame.h"
#include "widgets/TabPlayer.h"
#include "widgets/AudioItemMaxi.h"
#include "core/MediaManager.h"
#include "widgets/frameoptionsplayer.h"

class VuMeter;



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

    Button * btnoption;

    Button * btnclose;
    Label *labeltitle;

    TabPlayer *tabplayer;
    MediaManager *mediamanager;
    VuMeter *vumeter = nullptr;
    AudioItemMaxi* currentItem = nullptr;



    Button * btnstop;
    Label *labelnombre;
    Label *labeltiempo;


    Button * btnpause;
    Button * btnrewind;
    Button * btnforward;
    Slider * slider;

    bool m_userIsSeeking = false;
    double m_duration = 0.0;
    QString SecondToTime(double segundos);

    int m_deviceplay=0;
    int m_devicecue=0;


    FrameOptionsPlayer *frameoptionsplayer = nullptr; // si no esta a nullptr hace crack

public:

    explicit Player(QWidget *parent = nullptr);
    ~Player();

    void setTitle(QString title);
    QString title() const;

    void playItem(AudioItemMaxi *item);
    void pauseMain();
    void stopMain();
    bool setVolume(float volume);
    float volume() const;

    int devicePlay() const;
    void setDevicePlay(int device);

    int deviceCue() const;
    void setDeviceCue(int device);


    AudioItemMaxi* getCurrentItem() const {
        return currentItem;
    }


    protected:


    private slots:


    public slots:


    signals:
    void configurationChanged();


};





#endif // PLAYER_H
