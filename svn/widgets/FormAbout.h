#ifndef FORMABOUT_H
#define FORMABOUT_H



#include <QWidget>
#include <QTextEdit>


#include "widgets/FormProperties.h"
#include "widgets/button.h"


class FormAbout : public FormProperties {


   private:

     QTextEdit *texteditabout;
     Button *btnaccept;

    public:
       explicit FormAbout(QWidget *parent = nullptr);

      ~ FormAbout();




   protected:


   private slots:

   public slots:




};

#endif // FORMABOUT_H
