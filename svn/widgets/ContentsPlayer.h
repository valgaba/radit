#ifndef CONTENTSPLAYER_H
#define CONTENTSPLAYER_H




#include <QWidget>
#include <QVBoxLayout>
#include <QList>
#include <QFrame>
#include <QDebug>


#include "widgets/contentsbase.h"
//#include "widgets/menu.h"
#include "core/Clipboard.h"
//#include "widgets/AudioItemMaxi.h"
#include "widgets/ContentsMenu.h"

class Player;


class ContentsPlayer: public ContentsBase
{

    Q_OBJECT


private:

    QPoint mousePos;
    bool isCut;  //para las operaciones de cortar pegar
    ContentsMenu *contentsMenu = nullptr;
    Player *m_player = nullptr;

public:

    Clipboard &clipboard=Clipboard::instance();
    explicit ContentsPlayer(QWidget *parent = 0);
    ~ContentsPlayer();

    void selectAllItems();
    void deleteSelected();
    void copySelected();
    void cutSelected();
    void pasteClipboard();
    void applyColor(const QColor &color);

    void loadItems()   { /* TODO: implementar */ }
    void saveItems()   { /* TODO: implementar */ }
    void saveAsItems() { /* TODO: implementar */ }

    void setPlayer(Player *player);




protected:

    void contextMenuEvent(QContextMenuEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:


public slots:





};





#endif // CONTENTSPLAYER_H
