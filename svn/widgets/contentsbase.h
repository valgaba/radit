#ifndef CONTENTSBASE_H
#define CONTENTSBASE_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QDebug>


#include "core/Clipboard.h"
#include "widgets/AudioItemMaxi.h"
#include "core/MediaManager.h"


class ContentsBase: public QWidget{


    Q_OBJECT


private:


    QList<QWidget*>  list;
    QString formatTimeHhMmSsDd(double duration);
    MediaManager *mediamanager;



public:
    QVBoxLayout *layout;

    Clipboard &clipboard=Clipboard::instance();


    explicit ContentsBase(QWidget *parent = 0);
    ~ContentsBase();

    virtual AudioItemMaxi* createItem(AudioItemMaxi* item);


protected:

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;



private slots:


public slots:





};





#endif // CONTENTSBASE_H
