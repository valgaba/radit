#ifndef FORMPROPERTIES_H
#define FORMPROPERTIES_H



#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>


#include "widgets/AudioItem.h"
#include "widgets/container.h"
#include "widgets/frame.h"

class FormProperties : public QDialog {


    private:

        QVBoxLayout *layout;
        AudioItem *itembase;

        Container *container;
        QWidget * contents;
        Frame *frame;

        QHBoxLayout *downlayout;
        QGridLayout * toplayout;

    public:

       void addTopWidget(QWidget *widget, int row, int column);
       void addDownWidget(QWidget *widget);

       explicit FormProperties(QWidget *parent = nullptr);

      ~ FormProperties();



  protected:






  private slots:

  public slots:




};




#endif // FORMPROPERTIES_H
