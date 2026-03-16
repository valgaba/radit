#ifndef BUTTON_H
#define BUTTON_H

#include <QPushButton>


class Button: public QPushButton
{

    Q_OBJECT


    private:


    public:
 void SetIcon(QString file);


       explicit Button(QWidget *parent = 0);
       ~Button();




    protected:


    private slots:

    public slots:



};


#endif // BUTTON_H
