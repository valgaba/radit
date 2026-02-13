#ifndef AUDIOITEMMINI_H
#define AUDIOITEMMINI_H

#include <QHBoxLayout>
#include <QLabel>

#include "widgets/AudioItem.h"
#include "widgets/button.h"



class AudioItemMini: public AudioItem{


    Q_OBJECT


private:
    QLabel * nombre;



public:

    void setNameFile(const QString &NameFile) const;
     QHBoxLayout *layout;
     QLabel * duracion;
     Button *btnproperties;



     QLabel * hora;
    explicit AudioItemMini(QWidget *parent = 0);
    ~AudioItemMini();

    virtual AudioItem* copy() const = 0; // Método virtual puro

    protected:


    private slots:

    public slots:



};



#endif // AUDIOITEMMINI_H
