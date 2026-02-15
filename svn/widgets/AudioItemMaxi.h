#ifndef AUDIOITEMMAXI_H
#define AUDIOITEMMAXI_H


#include <QVBoxLayout>
#include <QHBoxLayout>


#include "widgets/frame.h"
#include "widgets/AudioItem.h"
#include "widgets/button.h"
#include "widgets/label.h"
#include "widgets/FrameColorItemMax.h"



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


public:

    void setNameFile(const QString &NameFile) const;


    explicit AudioItemMaxi(QWidget *parent = 0);
    ~AudioItemMaxi();

   virtual AudioItem* copy() const = 0; // Método virtual puro

    protected:


    private slots:

    public slots:



};



#endif // AUDIOITEMMAXI_H
