#ifndef AUDIOITEMFILE_H
#define AUDIOITEMFILE_H

#include <QHBoxLayout>
#include <QLabel>

#include "widgets/AudioItemMini.h"
#include "widgets/button.h"



class AudioItemFile: public AudioItemMini{


    Q_OBJECT


private:

     QHBoxLayout *layout;
     QLabel * duracion;
     Button *btnproperties;

public:

    QLabel * nombre;
    QLabel * hora;
    explicit AudioItemFile(QWidget *parent = 0);
    ~AudioItemFile();

     AudioItem* copy() const override;  // la virtual


    protected:



    private slots:

    public slots:



};




#endif // AUDIOITEMFILE_H
