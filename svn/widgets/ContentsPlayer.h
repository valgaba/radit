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


class ContentsPlayer: public ContentsBase
{

    Q_OBJECT


private:

    Menu *menu;
    QPoint mousePos;

    //opciones del menu contectual
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *propertiesAction;

    bool isCut;  //para las operaciones de cortar pegar



public:

    Clipboard &clipboard=Clipboard::instance();
    explicit ContentsPlayer(QWidget *parent = 0);
    ~ContentsPlayer();




protected:
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;


private slots:


public slots:





};





#endif // CONTENTSPLAYER_H
