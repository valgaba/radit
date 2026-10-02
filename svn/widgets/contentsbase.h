#ifndef CONTENTSBASE_H
#define CONTENTSBASE_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QDebug>


#include "core/Clipboard.h"
#include "widgets/AudioItemMaxi.h"
#include "core/MediaManager.h"
#include "widgets/Player.h"


class ContentsBase: public QWidget{


    Q_OBJECT


private:


    QList<QWidget*>  list;
   // QString formatTimeHhMmSsDd(double duration);
    MediaManager *mediamanager;


public:
    QVBoxLayout *layout;

    Clipboard &clipboard=Clipboard::instance();


    explicit ContentsBase(QWidget *parent = 0);
    ~ContentsBase();

    virtual AudioItemMaxi* createItem(AudioItemMaxi* item);
    virtual void deleteItem(AudioItemMaxi* item);
    Player* findPlayer() const;   //necesitamos el padre player para difentes operaciones

    AudioItemMaxi* findNextPlayItem(AudioItemMaxi* current);

     void clearItems();  // borrar los items al cargar una lista en los player

protected:

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void setTabName(const QString &filename); //cambia el nombre de los tab al cargar las listas



private slots:


public slots:

  signals:
   void requestPlayItem(AudioItemMaxi* item);



};





#endif // CONTENTSBASE_H
