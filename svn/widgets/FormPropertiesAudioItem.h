#ifndef FORMPROPERTIESAUDIOITEM_H
#define FORMPROPERTIESAUDIOITEM_H


#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "widgets/FormProperties.h"
#include "widgets/button.h"
#include "widgets/AudioItem.h"
#include "widgets/container.h"


class FormPropertiesAudioItem : public FormProperties {


    private:

        QVBoxLayout *layout;
        Button *btncancel;
        Button *btnaccept;
        AudioItem *itembase;

        Container *container;
        QWidget * contents;
       // QGridLayout * gridlayout;


        QLabel *namehour;
        QLineEdit *texthour;
        QLabel *namefile;
        QLineEdit *textfile;

        Button *btnfile;

        Container *grupcontainer;







    public:
       explicit FormPropertiesAudioItem(QWidget *parent = nullptr);

      ~ FormPropertiesAudioItem();

       void setItembase(AudioItem *itembase);
       AudioItem* getItembase();


  protected:






  private slots:

  public slots:




};





#endif // FORMPROPERTIESAUDIOITEM_H
