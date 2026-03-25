#ifndef CONTAINER_H
#define CONTAINER_H

#include <QScrollArea>
#include <QWidget>
#include <QVBoxLayout>

#include "widgets/ContentsPlayer.h"
#include "widgets/AudioItemMaxi.h"



class Container: public QScrollArea

{
    Q_OBJECT

    private:
     QVBoxLayout *layout;
     ContentsPlayer* contentsplayer;


    public:

           explicit Container(QWidget *parent = 0);
       ~Container();




    protected:






    private slots:

    public slots:

    signals:




};




#endif // CONTAINER_H
