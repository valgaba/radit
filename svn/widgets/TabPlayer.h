#ifndef TABPLAYER_H
#define TABPLAYER_H



//TabPlayer


#include "widgets/tab.h"
#include "widgets/menu.h"
class AudioItemMaxi;


class TabPlayer: public Tab
{

    Q_OBJECT


private:
        Menu *menu;


public:



    explicit TabPlayer(QWidget *parent = 0);
    ~TabPlayer();

 void closeTab(int index);


protected:
    void contextMenuEvent(QContextMenuEvent *event) override;


private slots:

public slots:

    signals:
        void requestPlayItem(AudioItemMaxi* item);

};







#endif // TABPLAYER_H
