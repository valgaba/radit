#ifndef AUDIOITEMFILEMAXI_H
#define AUDIOITEMFILEMAXI_H

#include <QHBoxLayout>
#include <QLabel>

#include "widgets/AudioItemMaxi.h"
#include "widgets/button.h"



class AudioItemFileMaxi: public AudioItemMaxi{


    Q_OBJECT


private:

     QHBoxLayout *layout;
     QLabel * duracion;
     Button *btnproperties;

public:

    QLabel * nombre;
    QLabel * hora;
    explicit AudioItemFileMaxi(QWidget *parent = 0);
    ~AudioItemFileMaxi();

     AudioItemMaxi* copy(QWidget* newParent) const override;  //  virtual


    protected:



    private slots:

    public slots:



};






#endif // AUDIOITEMFILEMAXI_H
