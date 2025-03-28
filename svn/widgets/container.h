#ifndef CONTAINER_H
#define CONTAINER_H

#include <QScrollArea>
#include <QWidget>
#include <QVBoxLayout>




class Container: public QScrollArea

{
    Q_OBJECT

    private:
     QVBoxLayout *layout;


    public:

           explicit Container(QWidget *parent = 0);
       ~Container();




    protected:






    private slots:

    public slots:



};




#endif // CONTAINER_H
