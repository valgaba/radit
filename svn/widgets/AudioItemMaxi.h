#ifndef AUDIOITEMMAXI_H
#define AUDIOITEMMAXI_H


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "widgets/frame.h"
#include "widgets/AudioItem.h"
#include "widgets/button.h"



class AudioItemMaxi: public AudioItem{


    Q_OBJECT


private:


    QVBoxLayout *layout; //general

    Frame *framecolor;
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
    Button * btnplaycue;
    Button * btnplay;


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
