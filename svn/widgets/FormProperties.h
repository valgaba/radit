#ifndef FORMPROPERTIES_H
#define FORMPROPERTIES_H



#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "widgets/button.h"
#include "widgets/AudioItem.h"
#include "widgets/container.h"


class FormProperties : public QDialog {


    private:

        QVBoxLayout *layout;
        Button *btncancel;
        Button *btnaccept;
        AudioItem *itembase;

        Container *container;
        QWidget * contents;
        QGridLayout * gridlayout;


        QLabel *namehour;
        QLineEdit *texthour;
        QLabel *namefile;
        QLineEdit *textfile;

        Button *btnfile;

        Container *grupcontainer;







    public:
       explicit FormProperties(QWidget *parent = nullptr);


      ~ FormProperties();

       void setItembase(AudioItem *itembase);
       AudioItem* getItembase();


  protected:






  private slots:

  public slots:




};




#endif // FORMPROPERTIES_H
