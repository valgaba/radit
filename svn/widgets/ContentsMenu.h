#ifndef CONTENTSMENU_H
#define CONTENTSMENU_H

// ContentsMenu.h
//#pragma once
#include <QMenu>
#include <QAction>
#include <QWidgetAction>
#include <QColor>
#include <QVector>

#include "widgets/menu.h"


class ContentsPlayer;
struct ColorButton;

class ContentsMenu : public Menu
{
    Q_OBJECT
public:
    explicit ContentsMenu(ContentsPlayer *parent);

    void showAt(const QPoint &pos);



    void setColorVisible(bool visible);
    void setSelectAllVisible(bool visible);
    void setCopyVisible(bool visible);
    void setCutVisible(bool visible);
    void setDeleteVisible(bool visible);
    void setPropertiesVisible(bool visible);
    void setPasteVisible(bool visible);

    void setLoadVisible(bool visible);
    void setSaveVisible(bool visible);
    void setSaveAsVisible(bool visible);

    void setnextVisible(bool visible);
    void setpurgeVisible(bool visible);
    void setloopVisible(bool visible);
    void setselectVisible(bool visible);


private:
    ContentsPlayer *player;

    // Acciones principales
    QAction *addMenuAction;

    QAction *nextItemAction;
    QAction *purgeItemAction;
    QAction *loopItemAction;
    QAction *selectItemAction;

    QAction *selectallAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;

    QAction *propertiesAction;
    QAction *colorAction;

    QAction *loadAction;
    QAction *saveAction;
    QAction *saveasAction;




    void setupAddMenu();
    void setupColorMenu();
    void setupActions();
    void connectActions();

    QAction* createAddAction();
    QAction* createColorAction();
    QIcon createColorIcon(const QColor &color);
};





#endif // CONTENTSMENU_H
