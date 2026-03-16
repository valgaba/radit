#ifndef AUDIOITEMNET_H
#define AUDIOITEMNET_H


#include <QHBoxLayout>
#include <QLabel>

#include "widgets/AudioItemMini.h"










class AudioItemNet: public AudioItemMini{


    Q_OBJECT


private:



public:


    explicit AudioItemNet(QWidget *parent = 0);
    ~AudioItemNet();

    // AudioItem* copy(QWidget* newParent) const override;  // la virtual


    protected:



    private slots:

    public slots:



};









#endif // AUDIOITEMNET_H
