#ifndef CONTENTSPLAYER_H
#define CONTENTSPLAYER_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QFrame>
#include <QDebug>


#include "widgets/contentsbase.h"
#include "widgets/menu.h"
#include "core/Clipboard.h"
#include "widgets/AudioItemMaxi.h"


class ContentsPlayer: public ContentsBase
{

    Q_OBJECT


private:

    Menu *menu;
    QPoint mousePos;

    //opciones del menu contectual

    QAction *selectallAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *propertiesAction;
    QAction *colorAction;


    bool isCut;  //para las operaciones de cortar pegar

   // AudioItemMaxi* createItem(AudioItemMaxi* item);

    static QIcon createColorIcon(const QColor &color);

public:

    Clipboard &clipboard=Clipboard::instance();
    explicit ContentsPlayer(QWidget *parent = 0);
    ~ContentsPlayer();




protected:

    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:


public slots:





};





#endif // CONTENTSPLAYER_H
