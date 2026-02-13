#ifndef AUDIOITEMFILEMINI_H
#define AUDIOITEMFILEMINI_H

#include <QHBoxLayout>
#include <QLabel>

#include "widgets/AudioItemMini.h"
#include "widgets/button.h"



class AudioItemFileMini: public AudioItemMini{


    Q_OBJECT


private:

     QHBoxLayout *layout;
     QLabel * duracion;
     Button *btnproperties;

public:

    QLabel * nombre;
    QLabel * hora;
    explicit AudioItemFileMini(QWidget *parent = 0);
    ~AudioItemFileMini();

     AudioItem* copy() const override;  //  virtual opcional viene del padre


    protected:



    private slots:

    public slots:



};





#endif // AUDIOITEMFILEMINI_H
